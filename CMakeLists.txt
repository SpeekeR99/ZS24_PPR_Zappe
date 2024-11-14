cmake_minimum_required(VERSION 3.24)
project(ZS24_PPR_Zappe)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

add_executable(
    ZS24_PPR_Zappe
    src/main.cpp
    src/merge_sort.h
    src/merge_sort.cpp
    src/bitonic_sort.h
    src/bitonic_sort.cpp
    src/statistics.h
    src/statistics.cpp
    src/dataloader.h
    src/dataloader.cpp
        src/utils.h)
