cmake_minimum_required(VERSION 3.0)
project(bfb)

set(bfb_sources
        src/benchmark.cpp
        include/bfb/benchmark.hpp
        include/bfb/benchmark.inl

        include/bfb/utils/chrono.hpp
        include/bfb/utils/moving_average.hpp
        include/bfb/utils/print_helpers.hpp
)

add_library(bfb ${bfb_sources})
target_include_directories(bfb PUBLIC include/)

option(BFB_INCLUDE_EXAMPLE "Includes the example target of the bigfatbench project." ON)

if (BFB_INCLUDE_EXAMPLE)
    add_subdirectory(example)
endif ()