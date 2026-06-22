cmake_minimum_required(VERSION 3.16)
project(qpdfImageEmbed VERSION 0.4.2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_subdirectory(src)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    include(CTest)

    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    configure_file(
        "${CMAKE_SOURCE_DIR}/CTestCustom.cmake.in"
        "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
        @ONLY
    )

    add_subdirectory(tests)
endif()
