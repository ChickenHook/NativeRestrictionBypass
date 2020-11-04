//
// Created by sascharoth on 21.09.20.
//


#include <string>
#include <jni.h>

class DiskLoader {
public:
    static int load(std::string path,JavaVM *);
};


