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

/**
 * Compute sum of elements in the array and sum of squared elements in the array
 * @param arr Array
 * @param sums Sum of elements
 * @param sums_sq Sum of squared elements
 * @param n Array size
 */
__kernel void reduce_sum(__global const double *arr, __global double *sums, __global double *sums_sq, const int n) {
    /* Get index */
    int gid = get_global_id(0);
    /* Get local index */
    int local_id = get_local_id(0);
    /* Get group index */
    int group_id = get_group_id(0);

    /* Local (shared) memory for partial sums */
    __local double partial_sums[256];
    __local double partial_sums_sq[256];

    /* Each thread loads one element into local memory */
    partial_sums[local_id] = (gid < n) ? arr[gid] : 0.0;
    partial_sums_sq[local_id] = (gid < n) ? arr[gid] * arr[gid] : 0.0;

    /* Synchronize */
    barrier(CLK_LOCAL_MEM_FENCE);

    /* Reduction in shared memory -- within a group */
    for (int stride = get_local_size(0) / 2; stride > 0; stride /= 2) {
        if (local_id < stride) {
            partial_sums[local_id] += partial_sums[local_id + stride];
            partial_sums_sq[local_id] += partial_sums_sq[local_id + stride];
        }

        /* Wait for all threads to finish */
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    /* Write result for this block to global memory */
    if (local_id == 0) {
        sums[group_id] = partial_sums[0];
        sums_sq[group_id] = partial_sums_sq[0];
    }
}
)";
#else
constexpr char kernel_source[] = R"(
/**
 * Merge two sorted sub-arrays
 * @param arr Array to be sorted
 * @param temp Temporary array
 * @param size Sub-array size (stride)
 * @param n Array size
 */
__kernel void merge(__global float *arr, __global float *temp, int size, int n) {
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
__kernel void abs_diff(__global const float *arr, __global float *diff, const float median) {
    /* Get index */
    int i = get_global_id(0);
    /* Compute absolute difference */
    diff[i] = fabs(arr[i] - median);
}

/**
 * Compute sum of elements in the array and sum of squared elements in the array
 * @param arr Array
 * @param sums Sum of elements
 * @param sums_sq Sum of squared elements
 * @param n Array size
 */
__kernel void reduce_sum(__global const float *arr, __global float *sums, __global float *sums_sq, const int n) {
    /* Get index */
    int gid = get_global_id(0);
    /* Get local index */
    int local_id = get_local_id(0);
    /* Get group index */
    int group_id = get_group_id(0);

    /* Local (shared) memory for partial sums */
    __local float partial_sums[256];
    __local float partial_sums_sq[256];

    /* Each thread loads one element into local memory */
    partial_sums[local_id] = (gid < n) ? arr[gid] : 0.0;
    partial_sums_sq[local_id] = (gid < n) ? arr[gid] * arr[gid] : 0.0;

    /* Synchronize */
    barrier(CLK_LOCAL_MEM_FENCE);

    /* Reduction in shared memory -- within a group */
    for (int stride = get_local_size(0) / 2; stride > 0; stride /= 2) {
        if (local_id < stride) {
            partial_sums[local_id] += partial_sums[local_id + stride];
            partial_sums_sq[local_id] += partial_sums_sq[local_id + stride];
        }

        /* Wait for all threads to finish */
        barrier(CLK_LOCAL_MEM_FENCE);
    }

    /* Write result for this block to global memory */
    if (local_id == 0) {
        sums[group_id] = partial_sums[0];
        sums_sq[group_id] = partial_sums_sq[0];
    }
}
)";
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
