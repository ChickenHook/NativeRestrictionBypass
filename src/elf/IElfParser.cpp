//
// Created by Sascha Roth on 11.11.19.
//

#include "IElfParser.h"
#include <utility>

ElfParser::IElfParser::IElfParser(const char *addr) {
    addr_ = addr;
}

