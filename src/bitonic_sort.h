#pragma once

#include <vector>
#include <cmath>
#include <algorithm>

/**
 * Merge function
 * Merges two sorted arrays (left and right half)
 * If final_merge is true, calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 * @param low Left index of the subarray
 * @param count Size of the subarray
 */
void bitonic_merge(std::vector<double>& arr, size_t low, size_t count);

/**
 * Bitonic sort function
 * Sorts the array and calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 */
void my_bitonic_sort(std::vector<double>& arr);
