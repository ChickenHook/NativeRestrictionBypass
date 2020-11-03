//
// Created by sascharoth on 21.09.20.
//

#pragma once
#include <vector>
#include <jni.h>


class MemLinker {
public:
    static int link(void* library,JavaVM *);

};
