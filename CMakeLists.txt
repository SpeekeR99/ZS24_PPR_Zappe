cmake_minimum_required(VERSION 3.21)
project(ZS24_PPR_Zappe)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)

# Set compiler specific flags
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Flags for MSVC - /W4 for warnings; /arch:AVX2 for AVX2; /D_USE_FLOAT to compile with single precision
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /arch:AVX2")
    # This is how you can compile with single precision instead of double precision
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /arch:AVX2 /D_USE_FLOAT")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Flags for GCC/Clang - -W* for warnings; -mavx2 for AVX2; -D_USE_FLOAT to compile with single precision
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -ltbb -mavx2")
    # This is how you can compile with single precision instead of double precision
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wpedantic -ltbb -mavx2 -D_USE_FLOAT")
endif()

# Find OpenCL
find_package(OpenCL REQUIRED)

# Include directories
include_directories(src)
include_directories(lib)

# Drawing library files from https://github.com/SmartCGMS/common.git
set(
    drawing_lib
    lib/drawing/Drawing.cpp
    lib/drawing/Drawing.h
    lib/drawing/IRenderer.h
    lib/drawing/SVGRenderer.cpp
    lib/drawing/SVGRenderer.h
)

# Add executable
add_executable(
    ZS24_PPR_Zappe
    src/main.cpp
    src/utils/utils.h
    src/utils/arg_parser.h
    src/utils/arg_parser.cpp
    src/dataloader/dataloader.h
    src/dataloader/dataloader.cpp
    src/calculations/computations.h
    src/calculations/computations.cpp
    src/calculations/cpu/cpu_comps.h
    src/calculations/cpu/cpu_comps.cpp
    src/calculations/cpu/merge_sort.h
    src/calculations/cpu/merge_sort.cpp
    src/calculations/gpu/gpu.h
    src/calculations/gpu/gpu.cpp
    src/calculations/gpu/gpu_comps.cpp
    src/calculations/gpu/gpu_comps.h
    ${drawing_lib}
    src/my_drawing/svg_generator.h
    src/my_drawing/svg_generator.cpp
)

# Link OpenCL
target_link_libraries(ZS24_PPR_Zappe OpenCL::OpenCL)
