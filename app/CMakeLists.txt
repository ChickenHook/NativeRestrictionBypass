cmake_minimum_required(VERSION 3.6)




### Libraries
ADD_LIBRARY( androidTest SHARED
    "src/main/cpp/interface.cpp"
)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

include_directories(
        "src/main/cpp/")
include_directories(
        "../src")
include_directories(
        "../src/include")

target_link_libraries( # Specifies the target library.
        androidTest
        ${PROJECT_NAME}
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})