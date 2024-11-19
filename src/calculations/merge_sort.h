#pragma once

#include <vector>
#include <algorithm>

#include <omp.h>
#include <execution>

#include "utils/utils.h"

/**
 * Merge function
 * Merges two sorted arrays (left and right half)
 * If final_merge is true, calculates the sum and sum of squares of the array elements
 * @param arr Array to be sorted
 * @param left Left index of the subarray
 * @param mid Middle index of the subarray
 * @param right Right index of the subarray
 */
void merge(std::vector<decimal> &arr, size_t left, size_t mid, size_t right);

/**
 * Modified merge sort function
 * Sorts the array and calculates the sum and sum of squares of the array elements
 * This function has to be implemented in here (.h), because of the template
 * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
 * @param policy Execution policy
 * @param arr Array to be sorted
 */
template <typename exec_policy>
void merge_sort(exec_policy policy, std::vector<decimal> &arr) {
    /* For each subarray size, basically a stride (bottom up approach) */
    for (size_t size = 1; size < arr.size(); size *= 2) {
        /* Prepare indices for std::for_each */
        std::vector<size_t> indices;
        for (size_t left = 0; left < arr.size(); left += 2 * size)
            indices.push_back(left);

        /* For each pair of sub-arrays -- left and right -- sort and merge them */
        std::for_each(policy, indices.begin(), indices.end(), [&](const auto left) {
            const size_t mid = std::min(left + size - 1, arr.size() - 1);
            const size_t right = std::min(left + 2 * size - 1, arr.size() - 1);

            /* Sort and merge the pair */
            merge(arr, left, mid, right);
        });
    }
}
