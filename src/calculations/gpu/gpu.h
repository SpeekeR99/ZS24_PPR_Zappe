#pragma once

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

/** Kernel source code -- basically "computations.cl" */
constexpr char kernel_source[] = R"(
__kernel void sum(__global const double *a, __global const double *b, __global double *c) {
    int i = get_global_id(0);
    c[i] = a[i] + b[i];
}
)";

/**
 * Initialize OpenCL platform
 * Tries to find NVIDIA platform, otherwise uses the first one
 * @return OpenCL platform
 */
cl::Platform init_platform();

/**
 * Initialize OpenCL device
 * Uses the first GPU device
 * @param platform OpenCL platform
 * @return OpenCL device
 */
cl::Device init_device(cl::Platform &platform);

/**
 * Load OpenCL program from source and build it
 * @param context Context
 * @param device Device
 * @param kernel_source Kernel source code
 * @return OpenCL program
 */
cl::Program load_program(cl::Context &context, cl::Device &device, const std::string &kernel_source);
