//
// Created by Sascha Roth on 25.10.19.
//

#pragma once

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <vector>

#if defined(__APPLE__) && defined(__MACH__)
#include <sys/ucontext.h>
#else
#include <ucontext.h>
#endif

#include <setjmp.h>

#include "logging.h"


namespace ChickenHook {
    namespace NativeBypass {

        /**
         * The chickenhook library.
         *
         * This framework is build for hooking native functions defined in libraries loaded in our process.
         * ChickenHook uses sigaction for managing jumps from the function to be hooked to our new function.
         * Therefore we write invalid code into the function to be hooked and wait for our registered signal handler.
         */
        class Resolve {


        public:

            static void* ResloveSymbol(void* lib, const std::string &symbol);

            static void* ResolveSymbol(const std::string &lib, const std::string &symbol);

        };
    }
}
