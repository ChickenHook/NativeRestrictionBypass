//
// Created by Sascha Roth on 03.11.19.
//

#pragma once


#include <string>
#include <memory>


namespace ElfParser {

    class IBinary {
    public:

        IBinary() {}

        /**
         * Open the elf file
         * @return true on success
         */
        virtual bool init(const char *addr) = 0;

        /**
         * close the elf file
         * @return true on success
         */
        virtual bool close() = 0;

        /**
         * Replace the given dependency name with a random generated name
         * @param libraryToReplace the library to be replaced
         * @return the new dependency's libname
         */
        virtual const std::string replaceDependency(const std::string &) = 0;

        /**
         * Generates a cpp file that contains implementations of all exported symbols of the given library.
         *
         * This cpp file automatically loads the correct library at runtime and calls the corresponding symbol
         *
         * @param libraryName the name of the library to generate the implementations for
         * @param outputFileName the output filename of the headers
         */
        virtual void
        generateSymbolHooks(const std::string &libraryName, const std::string &outputFileName) = 0;

        /**
         * Print some information about the binary
         */
        virtual void printInfo() = 0;

        virtual void addLibraryDependency(const std::string &library) = 0;


    protected:
        IBinary(const std::string &);

        /**
         * Returns the architecture.
         */
        virtual uint32_t getArchitecture() = 0;

    private:
    };

}