# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source _original_code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        ${PROJECT_NAME}

        # Sets the library as a shared library.
        STATIC

        # Provides a relative path to your source file(s).
        "MemLInker.cpp"
        "DiskLoader.cpp"
        "elf/Elf.cpp"
        "elf/Elf32Parser.cpp"
        "elf/Elf64Parser.cpp"
        "elf/IElfParser.cpp"
        "tools/MapsParser.cpp"
        "tools/HexDump.cpp"
        "tools/LoggingCallback.cpp"
        "Bypass.cpp"
        "logging.cpp"
        )



target_include_directories(${PROJECT_NAME} PUBLIC
        include/
        )
target_include_directories(${PROJECT_NAME} PRIVATE
        .
        )



set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)



install(TARGETS ${PROJECT_NAME}
        ARCHIVE
        DESTINATION ${CMAKE_SOURCE_DIR}/artifactsOut/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/lib/
        COMPONENT Libraries)
install(DIRECTORY include/ DESTINATION ${CMAKE_SOURCE_DIR}/artifactsOut/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}/include)
