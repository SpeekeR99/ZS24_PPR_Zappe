cmake_minimum_required(VERSION 3.24)
project(ZS24_PPR_Zappe)

set(CMAKE_CXX_STANDARD 20)

add_executable(
    ZS24_PPR_Zappe
    src/main.cpp
    src/merge_sort.h
    src/merge_sort.cpp
    src/statistics.h
    src/statistics.cpp
    src/dataloader.h
    src/dataloader.cpp
)
