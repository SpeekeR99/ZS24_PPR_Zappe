#pragma once

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

/** Kernel source code -- basically "computations.cl" */
#ifndef _USE_FLOAT
constexpr char kernel_source[] = R"(
__kernel void sum(__global const double *a, __global const double *b, __global double *c) {
    int i = get_global_id(0);
    c[i] = a[i] + b[i];
}

/**
 * Compute absolute difference between each element and the median
 * @param arr Array
 * @param diff Absolute difference between each element and the median
 * @param median Median of the array
 */
__kernel void abs_diff(__global const double *arr, __global double *diff, const double median) {
    /* Get index */
    int i = get_global_id(0);
    /* Compute absolute difference */
    diff[i] = fabs(arr[i] - median);
}
)";
#else
/* TODO: float kernel */
#endif

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
