#pragma once

#include <vector>
#include <algorithm>
#include <execution>

/**
 * Merge function
 * Merges two sorted arrays (left and right half)
 * If final_merge is true, calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 * @param left Left index of the subarray
 * @param mid Middle index of the subarray
 * @param right Right index of the subarray
 * @param sum Sum of the array elements
 * @param sum_sq Sum of squares of the array elements
 * @param final_merge Flag to indicate if this is the final merge
 */
void merge(std::vector<double> &arr, size_t left, size_t mid, size_t right, double &sum, double &sum_sq, bool final_merge = false);

/**
 * Modified merge sort function
 * Sorts the array and calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 * @param sum Sum of the array elements
 * @param sum_sq Sum of squares of the array elements
 */
void my_merge_sort(std::vector<double> &arr, double &sum, double &sum_sq);
