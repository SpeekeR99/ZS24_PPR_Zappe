#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#include <omp.h>
#include <immintrin.h>

#include "merge_sort.h"

/* This is the only file where I found OOP to be useful */

/**
 * Abstract class for computations
 * Defines the computation of MAD and CV (mean absolute deviation and coefficient of variance)
 * Also prepares virtual functions for computing the absolute difference and sums
 */
class computations {
public:
    /**
     * Default constructor
     */
    computations() = default;

    /**
     * Default destructor (virtual)
     */
    virtual ~computations() = default;

    /**
     * Compute the absolute difference between each element and the median
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    virtual void compute_abs_diff(const std::vector<double> &arr, double median, std::vector<double> &diff) const = 0;

    /**
     * Compute the sum and sum of squares of the array elements
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    virtual void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq) const = 0;

    /**
     * Compute the mean absolute deviation of a sorted array
     * @param arr Any array -- unsorted
     * @return Mean absolute deviation
     */
    [[nodiscard]] virtual double compute_mad(std::vector<double> &arr) const;

    /**
     * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
     * @param arr Array
     * @return Coefficient of variance
     */
    [[nodiscard]] virtual double compute_coef_var(const std::vector<double> &arr) const;
};

/**
 * Sequential computation class
 * Defines the computation of absolute difference and sums in a sequential manner
 */
class seq_comp : public computations {
public:
    /**
     * Default constructor
     */
    seq_comp() = default;

    /**
     * Default destructor
     */
    ~seq_comp() override = default;

    /**
     * Compute the absolute difference between each element and the median in a sequential manner
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    void compute_abs_diff(const std::vector<double> &arr, double median, std::vector<double> &diff) const override;

    /**
     * Compute the sum and sum of squares of the array elements in a sequential manner
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq) const override;
};

/**
 * Vectorized computation class
 * Defines the computation of absolute difference and sums in a vectorized manner
 */
class vec_comp : public computations {
public:
    /**
     * Default constructor
     */
    vec_comp() = default;

    /**
     * Default destructor
     */
    ~vec_comp() override = default;

    /**
     * Compute the absolute difference between each element and the median in a vectorized manner
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    void compute_abs_diff(const std::vector<double> &arr, double median, std::vector<double> &diff) const override;

    /**
     * Compute the sum and sum of squares of the array elements in a vectorized manner
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq) const override;
};
