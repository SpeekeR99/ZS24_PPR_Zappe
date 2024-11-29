#pragma once

#include <vector>
#include <cmath>

#include "utils/utils.h"

/* This, and the arg parser, are the only files where I found OOP to be useful */

/**
 * Abstract class for computations
 * Defines the computation of MAD and CV (mean absolute deviation and coefficient of variance)
 * Also prepares functions for computing the absolute difference and sums
 * Uses the static polymorphism technique (CRTP (Curiously Recurring Template Pattern)) to define the interface
 * Does not need to be instantiated, only inherited; does not need to have anything virtual
 * @tparam derived Derived class
 */
template<typename derived>
class computations {
public:
    /**
     * Compute the mean absolute deviation of a sorted array
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Any array -- unsorted
     * @return Mean absolute deviation
     */
    template <typename exec_policy>
    [[nodiscard]] decimal compute_mad(exec_policy policy, std::vector<decimal> &arr) {
        /* Sort the array for median calculation */
        static_cast<derived *>(this)->sort(policy, arr);

        /* Get the median */
        const auto median = static_cast<decimal>((arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0);

        /* Calculate the absolute differences from the median */
        std::vector<decimal> diff(arr.size());

        static_cast<derived *>(this)->compute_abs_diff(policy, arr, median, diff);

        /*
         * Since the array is "half-sorted", we can abuse it:
         * We start at the center, where we had the original median
         * We then move outwards, comparing elements from the left and right halves
         * We always pick the smaller of the two, move the corresponding pointer until we reach the new median
         */
        size_t left = diff.size() / 2 - 1, right = diff.size() / 2;
        decimal prev = 0, curr = 0;
        for (size_t i = 0; i <= diff.size() / 2; i++) {
            prev = curr;
            curr = diff[left] >= diff[right] ? diff[right++] : diff[left--];
        }

        return static_cast<decimal>((arr.size() & 1) ? curr : (prev + curr) / 2.0);
    }

    /**
     * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Array
     * @return Coefficient of variance
     */
    template <typename exec_policy>
    [[nodiscard]] decimal compute_coef_var(exec_policy policy, const std::vector<decimal> &arr) {
        /* Using the formula: sqrt((sum of squares - sum^2 / n) / n) / (sum / n) */
        const auto n = static_cast<double>(arr.size());

        decimal sum = 0, sum_sq = 0;
        static_cast<derived *>(this)->compute_sums(policy, arr, sum, sum_sq);

        return static_cast<decimal>(std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n));
    }
};