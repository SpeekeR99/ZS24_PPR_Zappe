#pragma once

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

/** Kernel source code -- basically "computations.cl" */
#ifndef _USE_FLOAT
constexpr char kernel_source[] = R"(
/**
 * Bitonic sort kernel
 * Uses sorting network to sort the array
 * @param arr Input array
 * @param stage Current stage
 * @param pass Current pass of the stage
 */
__kernel void bitonic_sort(__global double *arr, const uint stage, const uint pass) {
    /* Get index */
    int gid = get_global_id(0);

    /* Calculate pair distance and block width */
    uint pair_distance = 1 << (stage - pass);
    uint block_width = 2 * pair_distance;

    /* Calculate left and right indices */
    uint left_id = (gid & (pair_distance - 1)) + (gid >> (stage - pass)) * block_width;
    uint right_id = left_id + pair_distance;

    /* Load elements */
    double left = arr[left_id];
    double right = arr[right_id];

    /* Determine the direction */
    uint same_dir_block_width = gid >> stage;
    uint same_dir = same_dir_block_width & 0x1;

    /* Sort elements */
    if (same_dir) {
        uint temp = right_id;
        right_id = left_id;
        left_id = temp;
    }

    /* Write back -- swap if necessary */
    if (left < right) {
        arr[left_id] = left;
        arr[right_id] = right;
    } else {
        arr[left_id] = right;
        arr[right_id] = left;
    }
}

/**
 * Merge two sorted sub-arrays
 * UNUSED because it was slow (13.5 seconds) -- replaced by above bitonic_sort
 * @param arr Array to be sorted
 * @param temp Temporary array
 * @param size Sub-array size (stride)
 * @param n Array size
 */
__kernel void merge(__global double *arr, __global double *temp, const int size, const int n) {
    /* Get index */
    int gid = get_global_id(0);

    /* Calculate the total number of sub-arrays */
    const int num_sub_arrays = (n + 2 * size - 1) / (2 * size);

    /* Each work item processes multiple sub-arrays */
    for (int idx = gid; idx < num_sub_arrays; idx += get_global_size(0)) {
        /* Calculate indices for this sub-array */
        int left = idx * size * 2;
        int mid = min(left + size, n);
        int right = min(left + 2 * size, n);

        /* Copy data to temp */
        for (int i = left; i < right; i++)
            temp[i] = arr[i];

        /* Synchronize */
        barrier(CLK_GLOBAL_MEM_FENCE);

        /* Pointers to be moved around */
        int l = left, r = mid, t = left;

        /* Merge two sorted sub-arrays */
        while (l < mid && r < right)
            arr[t++] = temp[l] <= temp[r] ? temp[l++] : temp[r++];

        /* Copy the rest if there are any */
        while (l < mid)
            arr[t++] = temp[l++];
        while (r < right)
            arr[t++] = temp[r++];
    }
}

/**
 * Compute absolute difference between each element and the median
 * "my_" prefix, because for my integrated AMD GPU, "abs_diff" was already defined
 * @param arr Array
 * @param diff Absolute difference between each element and the median
 * @param median Median of the array
 */
__kernel void my_abs_diff(__global const double *arr, __global double *diff, const double median) {
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
 * Bitonic sort kernel
 * Uses sorting network to sort the array
 * @param arr Input array
 * @param stage Current stage
 * @param pass Current pass of the stage
 */
__kernel void bitonic_sort(__global float *arr, const uint stage, const uint pass) {
    /* Get index */
    int gid = get_global_id(0);

    /* Calculate pair distance and block width */
    uint pair_distance = 1 << (stage - pass);
    uint block_width = 2 * pair_distance;

    /* Calculate left and right indices */
    uint left_id = (gid & (pair_distance - 1)) + (gid >> (stage - pass)) * block_width;
    uint right_id = left_id + pair_distance;

    /* Load elements */
    float left = arr[left_id];
    float right = arr[right_id];

    /* Determine the direction */
    uint same_dir_block_width = gid >> stage;
    uint same_dir = same_dir_block_width & 0x1;

    /* Sort elements */
    if (same_dir) {
        uint temp = right_id;
        right_id = left_id;
        left_id = temp;
    }

    /* Write back -- swap if necessary */
    if (left < right) {
        arr[left_id] = left;
        arr[right_id] = right;
    } else {
        arr[left_id] = right;
        arr[right_id] = left;
    }
}

/**
 * Merge two sorted sub-arrays
 * UNUSED because it was slow (13.5 seconds) -- replaced by above bitonic_sort
 * @param arr Array to be sorted
 * @param temp Temporary array
 * @param size Sub-array size (stride)
 * @param n Array size
 */
__kernel void merge(__global float *arr, __global float *temp, const int size, const int n) {
    /* Get index */
    int gid = get_global_id(0);

    /* Calculate the total number of sub-arrays */
    const int num_sub_arrays = (n + 2 * size - 1) / (2 * size);

    /* Each work item processes multiple sub-arrays */
    for (int idx = gid; idx < num_sub_arrays; idx += get_global_size(0)) {
        /* Calculate indices for this sub-array */
        int left = idx * size * 2;
        int mid = min(left + size, n);
        int right = min(left + 2 * size, n);

        /* Copy data to temp */
        for (int i = left; i < right; i++)
            temp[i] = arr[i];

        /* Synchronize */
        barrier(CLK_GLOBAL_MEM_FENCE);

        /* Pointers to be moved around */
        int l = left, r = mid, t = left;

        /* Merge two sorted sub-arrays */
        while (l < mid && r < right)
            arr[t++] = temp[l] <= temp[r] ? temp[l++] : temp[r++];

        /* Copy the rest if there are any */
        while (l < mid)
            arr[t++] = temp[l++];
        while (r < right)
            arr[t++] = temp[r++];
    }
}

/**
 * Compute absolute difference between each element and the median
 * "my_" prefix, because for my integrated AMD GPU, "abs_diff" was already defined
 * @param arr Array
 * @param diff Absolute difference between each element and the median
 * @param median Median of the array
 */
__kernel void my_abs_diff(__global const float *arr, __global float *diff, const float median) {
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
 * @param _kernel_source Kernel source code
 * @return OpenCL program
 */
cl::Program load_program(cl::Context &context, cl::Device &device, const std::string &_kernel_source);
