#pragma once

#include <vector>
#include <algorithm>
#include <omp.h>

/**
 * Merge function
 * Merges two sorted arrays (left and right half)
 * If final_merge is true, calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 * @param left Left index of the subarray
 * @param mid Middle index of the subarray
 * @param right Right index of the subarray
 */
void merge(std::vector<double> &arr, size_t left, size_t mid, size_t right);

/**
 * Modified merge sort function
 * Sorts the array and calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 */
void merge_sort(std::vector<double> &arr);
