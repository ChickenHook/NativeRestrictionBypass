//
// Created by Sascha Roth on 11.11.19.
//

#pragma once

#include <functional>
#include <vector>

namespace ElfParser {

    class IElfParser {

    public:
        IElfParser(const char* addr);

        //virtual bool iterateSectionHeaders(const std::function<bool()>) = 0;
        virtual bool iterateSymbolTable(const std::vector<int> &types,
                                const std::function<bool(int type, char * symbolName, uint64_t st_value, uint8_t st_info)> function) = 0;

        virtual bool IterateRelocationTable(const std::vector<int> &types,
                                            const std::function<bool(int type, char * symbolName, uint64_t st_value, uint8_t st_info)> function) = 0;

        virtual bool iterateNeeded(const std::function<bool(char *libName)>) = 0;


    protected:
        const char* addr_;
    };

}