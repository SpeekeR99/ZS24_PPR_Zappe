#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <omp.h>
#include <iostream>
#include "merge_sort.h"

/**
 * Compute the sum and sum of squares of the array elements
 * @param arr Array
 * @param sum Sum of the array elements
 * @param sum_sq Sum of squares of the array elements
 */
void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq);

/**
 * Compute the mean absolute deviation of a sorted array
 * @param arr Any array -- unsorted
 * @return Mean absolute deviation
 */
double compute_mad(std::vector<double> &arr);

/**
 * Compute the coefficient of variance based on sum of X, sum of X^2 and number of elements
 * @param arr Array
 * @return Coefficient of variance
 */
double compute_coef_var(const std::vector<double> &arr);
