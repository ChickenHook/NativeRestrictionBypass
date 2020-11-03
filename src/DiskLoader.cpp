//
// Created by Sascha Roth on 21.09.20.
//

#include <android/log.h>
#include <unistd.h>
#include <vector>
#include "DiskLoader.h"
#include "MemLInker.h"
#include "Library.h"
#include <iostream>
#include <fstream>
#include <map>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <jni.h>


static std::map<std::string, Library> libraries;

/**
 *
 * @param path
 * @return -1 file doesn't exist
 *         -2 data size is too small
 *         -3 unable to link library
 */
int DiskLoader::load(std::string path, JavaVM *vm) {
    __android_log_print(ANDROID_LOG_DEBUG, "DiskLoader", "%s%s%s", "Going to load <", path.c_str(),
                        ">");

    struct stat sb;

    if (access(path.c_str(), F_OK) == -1) {
        return -1;
    }
    // read data
    int fd = open(path.c_str(), O_RDONLY);
    if (fd <= 0) {
        return -2;
    }
    fstat(fd, &sb);

    if (sb.st_size <= 0) {
        return -3;
    }
    void *addr = mmap(0, sb.st_size, PROT_WRITE | PROT_EXEC | PROT_READ, MAP_PRIVATE, fd, 0);

    // link
    if (MemLinker::link(addr, vm) != 0) {
        return -4;
    }

    return 0;
}
