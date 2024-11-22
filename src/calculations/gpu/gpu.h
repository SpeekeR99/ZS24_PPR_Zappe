#pragma once

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

/** Kernel source code -- basically "computations.cl" */
#ifndef _USE_FLOAT
constexpr char kernel_source[] = R"(
/**
 * Merge two sorted sub-arrays
 * @param arr Array to be sorted
 * @param temp Temporary array
 * @param size Sub-array size (stride)
 * @param n Array size
 */
__kernel void merge(__global double *arr, __global double *temp, int size, int n) {
    /* Get index */
    int gid = get_global_id(0);
    /* Equivalent of CPU sort 'for (size_t left = 0; left < arr.size(); left += 2 * size)' */
    int left = gid * size * 2;
    /* Mid and right indices could be out of bounds */
    int mid = min(left + size, n);
    int right = min(left + 2 * size, n);

    /* Pointers to be moved around */
    int l = left, r = mid, t = left;

    /* Merge two sorted sub-arrays */
    while (l < mid && r < right)
        temp[t++] = arr[l] <= arr[r] ? arr[l++] : arr[r++];

    /* Copy the rest if there are any */
    while (l < mid)
        temp[t++] = arr[l++];
    while (r < right)
        temp[t++] = arr[r++];

    /* Synchronize and copy back to the original array (in-place kind of) */
    for (int i = left; i < right; i++)
        arr[i] = temp[i];
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
