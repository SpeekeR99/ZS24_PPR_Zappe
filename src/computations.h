#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include <omp.h>
#include <immintrin.h>

#include "merge_sort.h"
#include "utils.h"

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
     * This function has to be impleneted in here (.h), because of the template
     * @param arr Any array -- unsorted
     * @return Mean absolute deviation
     */
    [[nodiscard]] decimal compute_mad(std::vector<decimal> &arr) {
        /* Sort the array for median calculation */
        merge_sort(arr);

        std::cout << (std::is_sorted(arr.begin(), arr.end()) ? "Sorted" : "Not sorted") << std::endl;

        /* Get the median */
        const auto median = (arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0;

        /* Calculate the absolute differences from the median */
        std::vector<decimal> diff(arr.size());
        static_cast<derived *>(this)->compute_abs_diff(arr, median, diff);

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

        return (arr.size() & 1) ? curr : (prev + curr) / 2.0;
    }

    /**
     * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
     * This function has to be impleneted in here (.h), because of the template
     * @param arr Array
     * @return Coefficient of variance
     */
    [[nodiscard]] decimal compute_coef_var(const std::vector<decimal> &arr) {
        /* Using the formula: sqrt((sum of squares - sum^2 / n) / n) / (sum / n) */
        const size_t n = arr.size();

        decimal sum = 0, sum_sq = 0;
        static_cast<derived *>(this)->compute_sums(arr, sum, sum_sq);

        return std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n);
    }
};

/**
 * Sequential computation class
 * Defines the computation of absolute difference and sums in a sequential manner
 * Uses the static polymorphism technique (CRTP (Curiously Recurring Template Pattern)) to define the interface
 */
class seq_comp : public computations<seq_comp> {
public:
    /**
     * Compute the absolute difference between each element and the median in a sequential manner
     * This is an actual implementation of the "abstract" function in the base class
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    static void compute_abs_diff(const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) ;

    /**
     * Compute the sum and sum of squares of the array elements in a sequential manner
     * This is an actual implementation of the "abstract" function in the base class
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    static void compute_sums(const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) ;
};

/**
 * Vectorized computation class
 * Defines the computation of absolute difference and sums in a vectorized manner
 * Uses the static polymorphism technique (CRTP (Curiously Recurring Template Pattern)) to define the interface
 */
class vec_comp : public computations<vec_comp> {
public:
    /**
     * Compute the absolute difference between each element and the median in a vectorized manner
     * This is an actual implementation of the "abstract" function in the base class
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    static void compute_abs_diff(const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) ;

    /**
     * Compute the sum and sum of squares of the array elements in a vectorized manner
     * This is an actual implementation of the "abstract" function in the base class
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    static void compute_sums(const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) ;
};
