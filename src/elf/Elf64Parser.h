//
// Created by sascharoth on 11.11.19.
//

#pragma once


#include <elf.h>
#include "IElfParser.h"

namespace ElfParser {

    class Elf64Parser : public IElfParser {


    public:
        Elf64Parser(const char* addr);


        bool iterateNeeded(const std::function<bool(char *)> function) override;

        bool iterateShdr(const std::function<bool(Elf64_Shdr *shdr)>& callback);


        template<class K>
        K *getSectionByName(const std::string &name);

        bool iterateSymbolTable(const std::vector<int> &types,
                            const std::function<bool(int, char *, uint64_t, uint8_t)> function) override;

        bool IterateRelocationTable(const std::vector<int> &types,
                                    const std::function<bool(int type, char *symbolName,
                                                            uint64_t st_value,
                                                            uint8_t st_info)> function) override;

        uint64_t SymAddrToFileOffset(uint64_t va);

        int elf_do_reloc(Elf64_Ehdr *hdr, Elf64_Rela *rel, Elf64_Shdr *reltab);

        char* GetSymbolNameByIndex(unsigned int index);
    };

}