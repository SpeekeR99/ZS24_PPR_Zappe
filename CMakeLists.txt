cmake_minimum_required(VERSION 3.21)
project(ZS24_PPR_Zappe)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -fopenmp -mavx2")
# This is how you can compile with single precision instead of double precision
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -fopenmp -mavx2 -D_USE_FLOAT")

include_directories(src)

add_executable(
    ZS24_PPR_Zappe
    src/main.cpp
    src/utils/utils.h
    src/utils/arg_parser.h
    src/utils/arg_parser.cpp
    src/dataloader/dataloader.h
    src/dataloader/dataloader.cpp
    src/calculations/merge_sort.h
    src/calculations/merge_sort.cpp
    src/calculations/computations.h
    src/calculations/computations.cpp
)
