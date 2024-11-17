cmake_minimum_required(VERSION 3.21)
project(ZS24_PPR_Zappe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -fopenmp -mavx2")

add_executable(
    ZS24_PPR_Zappe
    src/main.cpp
    src/arg_parser.h
    src/arg_parser.cpp
    src/dataloader.h
    src/dataloader.cpp
    src/merge_sort.h
    src/merge_sort.cpp
    src/computations.h
    src/computations.cpp
)
