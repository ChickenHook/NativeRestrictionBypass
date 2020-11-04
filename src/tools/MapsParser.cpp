//
// Created by sascharoth on 03.11.20.
//

#include "MapsParser.h"
#include <string>
#include <map>
#include <sstream>
#include <fstream>
#include "NativeBypass/logging.h"

constexpr size_t MAPS_NAME_BUFFER_SIZE(512);
constexpr size_t MAPS_FLAGS_BUFFER_SIZE(128);

namespace ChickenHook {

    std::map<std::string, LibraryEntry> MapsParser::generateLibraryMap() {
        log("ProcWatcher [-] generateLibraryMap");
        std::ifstream infile_("/proc/self/maps");
        std::map<std::string, LibraryEntry> libraryMap;
        if (!infile_.good()) {
            log(("ProcWatcher [-] generateLibraryMap [-] cannot read from maps, reopen file"));
            infile_.close();
            infile_.open("/proc/self/maps");
        }
        std::string lineRead;
        while (std::getline(infile_, lineRead)) {
            std::istringstream iss(lineRead);
            std::string line = iss.str();

            // get information about file
            void *start;
            void *end;
            char flags[MAPS_FLAGS_BUFFER_SIZE];
            long file_offset;
            size_t dev_major;
            size_t dev_minor;
            unsigned long inode;
            char name[MAPS_NAME_BUFFER_SIZE];
            std::memset(name, '0', sizeof name); // avoid invalid chars for JNI
            int res;
#ifdef __aarch64__
            // cppcheck-suppress invalidScanfArgType_int
        res = sscanf(line.c_str(), "%llx-%llx %31s %lx %lx:%lx %lu %512s", &start, &end, flags, &file_offset, &dev_major, &dev_minor, &inode, name);
#else
            // cppcheck-suppress invalidScanfArgType_int
            res = sscanf(line.c_str(), "%x-%x %31s %lx %x:%x %lu %512s", &start, &end, flags,
                         &file_offset, &dev_major, &dev_minor, &inode, name);
#endif
            if (res < 8 || res == EOF) {
                continue;
            }
            /*
             * If sscanf is unable to parse the name, the array is not filled with data.
             * This can lead to SEGV. We set to last possible position an end marker.
             * If data was added, an end marker was already set to a more sensible position.
             */
            name[MAPS_NAME_BUFFER_SIZE - 1] = '\0';
            if (strlen(name) <= 0) {
                log(("ProcWatcher [-] generateLibraryMap [-] Unable to parse name, continue"));
                continue;
            }
            // fill address entry
            AddressEntry addressEntry;
            addressEntry.start = start;
            addressEntry.end = end;
            addressEntry.flags = std::string(flags);
            addressEntry.file_offset = file_offset;
            addressEntry.dev_major = dev_major;
            addressEntry.dev_minor = dev_minor;
            addressEntry.inode = inode;
            addressEntry.crc32 = 0;


            // fill library entry
            LibraryEntry libraryEntry;
            if (libraryMap.find(name) != libraryMap.end()) {
                libraryEntry = libraryMap[name];
                libraryEntry.addressEntries.push_back(addressEntry);
                libraryMap[name] = libraryEntry;

            } else {
                libraryEntry.name = name;
                libraryEntry.addressEntries.push_back(addressEntry);
                libraryMap.insert(std::make_pair(name, libraryEntry));
                //logv("ProcWatcher [-] generateLibraryMap [-] Inserted %d", libraryMap.size());
            }


            //logv("ProcWatcher [-] generateLibraryMap [-] Found library: %s %d", name, libraryEntry.addressEntries.size());

        }
        infile_.clear();                 // clear fail and eof bits
        infile_.seekg(0, std::ios::beg); // back to the start!
        return libraryMap;
    }
}