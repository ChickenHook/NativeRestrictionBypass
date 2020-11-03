//
// Created by Sascha Roth on 03.11.19.
//

#pragma once


#include "IBinary.h"
#include <fstream>
#include <vector>
#include "IElfParser.h"

namespace ElfParser {


    class Elf : public IBinary {
    public:
        Elf();

    public:

        /**
         * Determine if the given magic equals elf magic
         * @param magic the magic bytes
         * @return true if magic indicates an elf file
         */
        static bool isElf(const char * addr);

        /**
         * Open the elf file
         * @return true on success
         */
        virtual bool init(const char *addr) override;

        /**
         * close the elf file
         * @return true on success
         */
        virtual bool close() override;

        /**
         * Replace the given dependency name with a random generated name
         * @param libraryToReplace the library to be replaced
         * @return the new dependency's libname
         */
        virtual const std::string replaceDependency(const std::string &libraryToReplace);

        bool iterateSymbolTable(const std::vector<int> &types,
                                             const std::function<bool(int, char *, uint64_t,
                                                                      uint8_t)> function);

        bool IterateRelocationTable(const std::vector<int> &types,
                                    const std::function<bool(int, char *, uint64_t,
                                                                      uint8_t)> function);

        /**
         * Print some information about the binary
         */
        virtual void printInfo();

        /**
         * NOT IMPLEMENTED YET
         * @param library
         */
        virtual void addLibraryDependency(const std::string &library);

        /**
         * Generates a cpp file that contains implementations of all exported symbols of the given library.
         *
         * This cpp file automatically loads the correct library at runtime and calls the corresponding symbol
         *
         * @param libraryName the name of the library to generate the implementations for
         * @param outputFileName the output filename of the headers
         */
        void generateSymbolHooks(const std::string &libraryName, const std::string &outputFileName);


    protected:
    /**
     * Returns the architecture.
     */
    uint32_t getArchitecture() override;

private:
    const char* addr;
    std::unique_ptr<IElfParser> _elf_parser;


    /**
     * @return true if 64 bit
     */
        bool is64();


        /**
         * Tells us the endian format
         * @return true if endian format is little endian
         */
        bool isLittleEndian();

    };

}