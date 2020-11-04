//
// Created by Sascha Roth on 21.09.20.
//

#include "MemLInker.h"
#include <vector>
#include <android/log.h>
#include <elf/Elf.h>
#include <elf.h>
#include "tools/LoggingCallback.h"
#include <jni.h>

constexpr char *JNI_ONLOAD("JNI_OnLoad");

#define JNI_ONLOAD_SIG jint (*)(JavaVM *, void *)

int MemLinker::link(void *library, JavaVM *vm) {

    log("MemLinker [+] %s%p%s", "link data <", library, ">");


    // cast to char *

    const char *addr = (const char *) library;

    // parse elf
    if (!ElfParser::Elf::isElf(addr)) {
        log("%s", "MemLinker [-] Not an elf file!");
        return -1;
    }

    ElfParser::Elf ElfFile;
    ElfFile.init(addr);
    ElfFile.printInfo();


    // fix relocations
    std::vector<int> types;

    if (!ElfFile.IterateRelocationTable(types, [](int type,
                                                  char *symbolName,
                                                  uint64_t st_value,
                                                  uint8_t st_info) {
                                        log("MemLinker [+] Got relocation: <%s> type<%lx> st_value<%lx> st_info<%lx> st_other<%lx>",
                                            symbolName, type, st_value, st_info);

                                        // https://wiki.osdev.org/ELF_Tutorial
        return true;
    })) {

    }

    // get symbol addresses
    types.clear();
    types.push_back(SHT_SYMTAB);
    types.push_back(SHT_DYNSYM);
    bool JNI_OnLoad_found = false;
    void *JNI_OnLoad_addr = nullptr;
    if (!ElfFile.iterateSymbolTable(types,
                                    [&JNI_OnLoad_found, &JNI_OnLoad_addr, addr](int type,
                                                                                char *symbolName,
                                                                                uint64_t st_value,
                                                                                uint8_t st_info) {

//                                        log("MemLinker [+] Got symbol: <%s> type<%lx> st_value<%lx> st_info<%lx> st_other<%lx>",
//                                            symbolName, type, st_value, st_info);
                                        if (strcmp(JNI_ONLOAD, symbolName) == 0) {
                                            JNI_OnLoad_found = true;
                                        }
                                        JNI_OnLoad_addr = (void *) (addr + st_value);
                                        return true;
                                    })) {
        log("MemLinker [-] Error while iterate symbols, abort...");
        return -2;
    }

    // call constructor

    // call JNI_OnLoad
    log("MemLinker [+] call JNI_OnLoad");
    if (!JNI_OnLoad_found || JNI_OnLoad_addr == addr) {
        log("MemLinker [-] Unable to find JNI_OnLoad...");
        return -3;
    }

    auto JNI_OnLoad_fun = (JNI_ONLOAD_SIG) JNI_OnLoad_addr;
    JNI_OnLoad_fun(vm, nullptr);

    return 0;
}
