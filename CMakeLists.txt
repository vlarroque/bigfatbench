cmake_minimum_required(VERSION 3.15)
project(bfb)


add_library(bfb INTERFACE include/bfb/benchmark.hpp)
target_include_directories(bfb INTERFACE include/)

option(BFB_INCLUDE_EXAMPLE "Includes the example target of the bigfatbench project." ON)

if (BFB_INCLUDE_EXAMPLE)
    add_subdirectory(example)
endif ()