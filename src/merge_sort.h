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
 */
void merge(std::vector<double> &arr, size_t left, size_t mid, size_t right);

/**
 * Modified merge sort function
 * Sorts the array and calculates the sum and sum of squares of the array elements
 * @tparam execution_policy Execution policy type -- because of this template, the function has to be defined here
 * @param arr Array to be sorted
 * @param policy Execution policy
 */
template <typename execution_policy>
void my_merge_sort(std::vector<double> &arr, execution_policy policy) {
    for (size_t size = 1; size < arr.size(); size *= 2) {
        std::vector<size_t> indices;
        for (size_t i = 0; i < arr.size(); i += 2 * size)
            indices.push_back(i);

        std::for_each(policy, indices.begin(), indices.end(), [&](size_t left) {
            size_t mid = std::min(left + size - 1, arr.size() - 1);
            size_t right = std::min(left + 2 * size - 1, arr.size() - 1);

            /* Merge */
            merge(arr, left, mid, right);
        });
    }
}
