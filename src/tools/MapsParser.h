//
// Created by sascharoth on 03.11.20.
//

#pragma once

#include <string>
#include <map>
#include <vector>

namespace ChickenHook {

/**
 * Represents one line of /proc/self/maps
 */
    struct AddressEntry {
        void *start = 0;
        void *end = 0;
        std::string flags = "";
        long file_offset = 0;
        size_t dev_major = 0;
        size_t dev_minor = 0;
        unsigned long inode = 0;
        uint64_t crc32 = 0;
        bool checksumCalculated = false;
    } typedef AddressEntry;

/**
 * Represents one line of /proc/self/maps
 */
    struct LibraryEntry {
    public:
        std::string name;
        std::vector<AddressEntry> addressEntries;
    } typedef LibraryEntry;

    class MapsParser {
    public:
        static std::map<std::string, LibraryEntry> generateLibraryMap();
    };
}