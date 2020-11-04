//
// Created by sascharoth on 11.11.19.
//

#pragma once


#include <elf.h>
#include "IElfParser.h"

namespace ElfParser {

    class Elf32Parser : public IElfParser {


    public:
        Elf32Parser(const char*);


        bool iterateNeeded(const std::function<bool(char *)> function) override;

        bool iterateShdr(const std::function<bool(Elf32_Shdr *shdr)>& callback);


        template<class K>
        K *getSectionByName(const std::string &name);

        bool iterateSymbolTable(const std::vector<int> &types,
                                const std::function<bool(int, char *, uint64_t, uint8_t)> function) override;

        bool IterateRelocationTable(const std::vector<int> &types,
                                    const std::function<bool(int type, char *symbolName,
                                                             uint64_t st_value,
                                                             uint8_t st_info)> function) override;

        uint32_t SymAddrToFileOffset(uint32_t va);

    };

}