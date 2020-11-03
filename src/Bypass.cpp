
#include "NativeBypass/bypass.h"
#include <string>
#include <elf.h>
#include <tools/MapsParser.h>
#include "elf/Elf.h"

namespace ChickenHook {
    namespace NativeBypass {


        void *Resolve::ResolveSymbol(const std::string &lib, const std::string &symbol) {
            auto maps = MapsParser::generateLibraryMap();
            void *start = nullptr;
            std::string LibPath;
            for (auto &entry : maps) {
                if (entry.second.name.find(lib) != std::string::npos) {
                    LibPath = entry.second.name;
                    for (auto &addrEntry : entry.second.addressEntries) {
                        if (addrEntry.start < start || start == nullptr) {
                            start = addrEntry.start;
                        }
                        log("Library: %s <%p, %p>", entry.second.name.c_str(), addrEntry.start,
                            addrEntry.end);
                    }
                }
            }

            // parse elf
            if (!ElfParser::Elf::isElf((const char *) start)) {
                log("%s", "Resolve [-] Not an elf file!");
                return nullptr;
            }
            ElfParser::Elf ElfFile;
            ElfFile.init((const char *) start);
            ElfFile.printInfo();

            std::ifstream is(LibPath, std::ios::binary);
            if (is.good()) {
                std::vector<char> buffer(std::istreambuf_iterator<char>(is), {});
                void *RelativeStart = &buffer[0];
                void *RelativeSymbolAddr1 = ResloveSymbol(&buffer[0], symbol);
                if (RelativeSymbolAddr1 == nullptr) {
                    return nullptr;
                }
                char *Address =
                        ((char *) RelativeSymbolAddr1 - (char *) RelativeStart) + (char *) start;
                log("Found Symbol: %s:%s => %p", lib.c_str(), symbol.c_str(), Address);
                return Address;
            } else {
                log("Cannot open library %s", lib.c_str());
            }

            return nullptr;
        }

        void *Resolve::ResloveSymbol(void *lib, const std::string &symbol) {

            log("Resolve [+] %s%p%s", "link data <", lib, ">");


            // cast to char *

            const char *addr = (const char *) lib;

            // parse elf
            if (!ElfParser::Elf::isElf(addr)) {
                log("%s", "Resolve [-] Not an elf file!");
                return nullptr;
            }

            ElfParser::Elf ElfFile;
            ElfFile.init(addr);
            ElfFile.printInfo();


            // get symbol addresses
            std::vector<int> types;
            types.clear();
            types.push_back(SHT_SYMTAB);
            types.push_back(SHT_DYNSYM);

            void *FunAddr = nullptr;
            if (!ElfFile.iterateSymbolTable(types,
                                            [&FunAddr, &symbol, addr](int type,
                                                                      char *symbolName,
                                                                      uint64_t st_value,
                                                                      uint8_t st_info) {
//                                                log("Symbol: %s", symbolName);
                                                if (strcmp(symbol.c_str(), symbolName) == 0) {
                                                    FunAddr = (void *) (addr + st_value);
                                                }
                                                return true;
                                            })) {
                log("Resolve [-] Error while iterate symbols, abort...");
                return nullptr;
            }

            // Get address
            log("Resolve [+] found symbol at address %p", FunAddr);
            return FunAddr;
        }
    }
}
