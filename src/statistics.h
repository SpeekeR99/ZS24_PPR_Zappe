#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <execution>
#include <iostream>
#include "merge_sort.h"

/**
 * Compute the sum and sum of squares of the array elements
 * @tparam execution_policy Execution policy type -- because of this template, the function has to be defined here
 * @param arr Array
 * @param sum Sum of the array elements
 * @param sum_sq Sum of squares of the array elements
 * @param policy Execution policy
 */
template <typename execution_policy>
void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq, execution_policy policy) {
    sum = 0;
    sum_sq = 0;

    std::for_each(policy, arr.begin(), arr.end(), [&](double i) {
        sum += i;
        sum_sq += i * i;
    });
}

/**
 * Compute the mean absolute deviation of a sorted array
 * @tparam execution_policy Execution policy type -- because of this template, the function has to be defined here
 * @param arr Any array -- unsorted
 * @param policy Execution policy
 * @return Mean absolute deviation
 */
template <typename execution_policy>
double compute_mad(std::vector<double> &arr, execution_policy policy) {
    my_merge_sort(arr, policy);

    std::cout << ((std::is_sorted(arr.begin(), arr.end())) ? "Sorted" : "ERR: NOT SORTED!!!") << std::endl;

    auto median = (arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0;

    std::vector<double> diff(arr.size());
    for (size_t i = 0; i < arr.size(); i++)
        diff[i] = std::abs(arr[i] - median);

    /* Since the array is "half-sorted", we can abuse it */
    size_t left = diff.size() / 2 - 1, right = diff.size() / 2;
    double prev = 0, curr = 0;
    for (size_t i = 0; i <= diff.size() / 2; i++) {
        prev = curr;
        if (diff[left] >= diff[right])
            curr = diff[right++];
        else
            curr = diff[left--];
    }

    if (arr.size() & 1)  /* Odd number */
        return curr;
    else
        return (prev + curr) / 2.0;
}

/**
 * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
 * @tparam execution_policy Execution policy type -- because of this template, the function has to be defined here
 * @param arr Array
 * @param policy Execution policy
 * @return Coefficient of variance
 */
template <typename execution_policy>
double compute_coef_var(const std::vector<double> &arr, execution_policy policy) {
    size_t n = arr.size();

    double sum = 0, sum_sq = 0;
    compute_sums(arr, sum, sum_sq, policy);

    return std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n);
}
