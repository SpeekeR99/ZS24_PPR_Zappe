#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <variant>

#include <omp.h>
#include <execution>
#include <immintrin.h>

#include "calculations/merge_sort.h"
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
        merge_sort(policy, arr);

        std::cout << (std::is_sorted(arr.begin(), arr.end()) ? "Sorted" : "Not sorted") << std::endl;

        /* Get the median */
        const auto median = (arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0;

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

        return (arr.size() & 1) ? curr : (prev + curr) / 2.0;
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
        const size_t n = arr.size();

        decimal sum = 0, sum_sq = 0;
        static_cast<derived *>(this)->compute_sums(policy, arr, sum, sum_sq);

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
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    template<typename exec_policy>
    static void compute_abs_diff(exec_policy policy, const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) {
        /* Calculate the absolute differences from the median */
        std::for_each(policy, arr.begin(), arr.end(), [&](const auto &val) {
            diff[&val - &arr[0]] = std::abs(val - median);
        });
    }

    /**
     * Compute the sum and sum of squares of the array elements in a sequential manner
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    template<typename exec_policy>
    static void compute_sums(exec_policy policy, const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
        /* Prepare for parallelism */
        const auto max_num_threads = std::thread::hardware_concurrency();
        const auto chunk_size = arr.size() / max_num_threads;
        std::vector<decimal> sums(max_num_threads, 0);
        std::vector<decimal> sums_sq(max_num_threads, 0);
        std::vector<size_t> chunk_indices(max_num_threads);
        std::iota(chunk_indices.begin(), chunk_indices.end(), 0);

        /* Calculate the sums */
        std::for_each(policy, chunk_indices.begin(), chunk_indices.end(), [&](const auto &i) {
            const size_t start = i * chunk_size;
            /* Final thread may have to handle a little more elements */
            const size_t end = (i == max_num_threads - 1) ? arr.size() : start + chunk_size;

            /* Calculate the sum and sum of squares */
            for (size_t j = start; j < end; j++) {
                sums[i] += arr[j];
                sums_sq[i] += arr[j] * arr[j];
            }
        });

        /* Combine the sums (reduce) */
        for (size_t i = 0; i < max_num_threads; i++) {
            sum += sums[i];
            sum_sq += sums_sq[i];
        }
    }
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
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    template<typename exec_policy>
    static void compute_abs_diff(exec_policy policy, const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) {
        const size_t n = arr.size();

        /* How many decimals can be processed at once */
        #ifndef _USE_FLOAT
        const size_t vec_capacity = sizeof(__m256d) / sizeof(decimal);
        #else
        const size_t vec_capacity = sizeof(__m256) / sizeof(decimal);
        #endif

        /* Load the median into an AVX2 register */
        #ifndef _USE_FLOAT
        __m256d median_vec = _mm256_set1_pd(median);
        #else
        __m256 median_vec = _mm256_set1_ps(median);
        #endif

        /* Prepare for parallelism */
        std::vector<size_t> indices(n / vec_capacity);
        for (size_t i = 0; i < indices.size(); i++)
            indices[i] = i * vec_capacity;

        /* Process 4 doubles at a time */
        std::for_each(policy, indices.begin(), indices.end(), [&](const auto &i) {
            #ifndef _USE_FLOAT
            /* Load 4 doubles into an AVX2 register */
            __m256d arr_vec = _mm256_loadu_pd(&arr[i]);

            /* Subtract the median from the array */
            __m256d diff_vec = _mm256_sub_pd(arr_vec, median_vec);
            /* Absolute value */
            __m256d sign_bit = _mm256_set1_pd(-0.0);
            diff_vec = _mm256_andnot_pd(sign_bit, diff_vec);

            /* Store the result */
            _mm256_storeu_pd(&diff[i], diff_vec);
            #else
            /* Load 8 floats into an AVX2 register */
            __m256 arr_vec = _mm256_loadu_ps(&arr[i]);

            /* Subtract the median from the array */
            __m256 diff_vec = _mm256_sub_ps(arr_vec, median_vec);
            /* Absolute value */
            __m256 sign_bit = _mm256_set1_ps(-0.0);
            diff_vec = _mm256_andnot_ps(sign_bit, diff_vec);

            /* Store the result */
            _mm256_storeu_ps(&diff[i], diff_vec);
            #endif
        });

        /* Handle the remaining elements (if array size is not a multiple of 4) */
        for (size_t i = n - n % vec_capacity; i < n; i++)
            diff[i] = std::abs(arr[i] - median);
    }

    /**
     * Compute the sum and sum of squares of the array elements in a vectorized manner
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    template<typename exec_policy>
    static void compute_sums(exec_policy policy, const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
        const size_t n = arr.size();

        /* How many decimals can be processed at once */
        #ifndef _USE_FLOAT
        const size_t vec_capacity = sizeof(__m256d) / sizeof(decimal);
        #else
        const size_t vec_capacity = sizeof(__m256) / sizeof(decimal);
        #endif

        /* Prepare for parallelism */
        const auto max_num_threads = std::thread::hardware_concurrency();
        auto chunk_size = arr.size() / max_num_threads;
        /* Make sure that chunk_size is a multiple of 4 */
        chunk_size = chunk_size - chunk_size % vec_capacity;
        std::vector<decimal> sums(max_num_threads, 0);
        std::vector<decimal> sums_sq(max_num_threads, 0);
        std::vector<size_t> chunk_indices(max_num_threads);
        std::iota(chunk_indices.begin(), chunk_indices.end(), 0);

        /* Calculate the sums */
        std::for_each(policy, chunk_indices.begin(), chunk_indices.end(), [&](const auto &i) {
            const size_t start = i * chunk_size;
            /* Here we are sure that chunk_size is a multiple of 4, so last thread does not have to handle more work */
            const size_t end = start + chunk_size;

            /* Initialize an AVX2 register for sums with zeros */
            #ifndef _USE_FLOAT
            __m256d sum_vec = _mm256_setzero_pd();
            __m256d sum_sq_vec = _mm256_setzero_pd();
            #else
            __m256 sum_vec = _mm256_setzero_ps();
            __m256 sum_sq_vec = _mm256_setzero_ps();
            #endif

            /* Process 4 doubles at a time */
            for (size_t j = start; j < end; j += vec_capacity) {
                #ifndef _USE_FLOAT
                /* Load 4 doubles into an AVX2 register */
                __m256d arr_vec = _mm256_loadu_pd(&arr[j]);

                /* Compute the sums */
                sum_vec = _mm256_add_pd(sum_vec, arr_vec);
                sum_sq_vec = _mm256_add_pd(sum_sq_vec, _mm256_mul_pd(arr_vec, arr_vec));
                #else
                /* Load 8 floats into an AVX2 register */
                __m256 arr_vec = _mm256_loadu_ps(&arr[j]);

                /* Compute the sums */
                sum_vec = _mm256_add_ps(sum_vec, arr_vec);
                sum_sq_vec = _mm256_add_ps(sum_sq_vec, _mm256_mul_ps(arr_vec, arr_vec));
                #endif
            }

            /* Extract the sums from the AVX2 registers */
            std::vector<decimal> sum_arr(vec_capacity);
            std::vector<decimal> sum_sq_arr(vec_capacity);
            #ifndef _USE_FLOAT
            _mm256_storeu_pd(sum_arr.data(), sum_vec);
            _mm256_storeu_pd(sum_sq_arr.data(), sum_sq_vec);
            #else
            _mm256_storeu_ps(sum_arr, sum_vec);
            _mm256_storeu_ps(sum_sq_arr, sum_sq_vec);
            #endif

            /* Combine the sums */
            for (size_t j = 0; j < vec_capacity; j++) {
                sums[i] += sum_arr[j];
                sums_sq[i] += sum_sq_arr[j];
            }
        });

        /* Combine the sums */
        for (size_t i = 0; i < max_num_threads; i++) {
            sum += sums[i];
            sum_sq += sums_sq[i];
        }

        /* Handle the remaining elements (if array size or chunk size were not a multiple of 4) */
        for (size_t i = chunk_size * max_num_threads; i < n; i++) {
            sum += arr[i];
            sum_sq += arr[i] * arr[i];
        }
    }
};
