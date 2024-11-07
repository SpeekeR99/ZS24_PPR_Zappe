#pragma once

#include <vector>
#include <cmath>

/**
 * Compute the mean absolute deviation of a sorted array
 * @param arr Sorted array
 * @return Mean absolute deviation
 */
double compute_mad(const std::vector<double>& arr);

/**
 * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
 * @param sum Sum of the array elements
 * @param sum_sq Sum of squares of the array elements
 * @param n Number of elements
 * @return Coefficient of variance
 */
double compute_coef_var(double sum, double sum_sq, size_t n);
