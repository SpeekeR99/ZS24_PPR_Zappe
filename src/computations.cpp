#include "computations.h"

void seq_comp::compute_abs_diff(const std::vector<decimal> &arr, const decimal median, std::vector<decimal> &diff) {
    /* Calculate the absolute differences from the median */
    #pragma omp parallel for default(none) shared(arr, diff, median)
    for (size_t i = 0; i < arr.size(); i++)
        diff[i] = std::abs(arr[i] - median);
}

void seq_comp::compute_sums(const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
    /* Prepare for parallelism */
    const auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    const auto chunk_size = arr.size() / max_num_threads;
    std::vector<decimal> sums(max_num_threads, 0);
    std::vector<decimal> sums_sq(max_num_threads, 0);

    /* Calculate the sums */
    #pragma omp parallel for default(none) shared(arr, sums, sums_sq, max_num_threads, chunk_size)
    for (size_t i = 0; i < max_num_threads; i++) {
        const size_t start = i * chunk_size;
        /* Final thread may have to handle a little more elements */
        const size_t end = (i == max_num_threads - 1) ? arr.size() : start + chunk_size;

        /* Calculate the sum and sum of squares */
        for (size_t j = start; j < end; j++) {
            sums[i] += arr[j];
            sums_sq[i] += arr[j] * arr[j];
        }
    }

    /* Combine the sums (reduce) */
    for (size_t i = 0; i < max_num_threads; i++) {
        sum += sums[i];
        sum_sq += sums_sq[i];
    }
}

void vec_comp::compute_abs_diff(const std::vector<decimal> &arr, const decimal median, std::vector<decimal> &diff) {
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

    /* Process 4 doubles at a time */
    #pragma omp parallel for default(none) shared(arr, diff, median_vec, n, vec_capacity)
    for (size_t i = 0; i <= n - vec_capacity; i += vec_capacity) {
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
    }

    /* Handle the remaining elements (if array size is not a multiple of 4) */
    for (size_t i = n - n % vec_capacity; i < n; i++)
        diff[i] = std::abs(arr[i] - median);
}

void vec_comp::compute_sums(const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
    const size_t n = arr.size();

    /* How many decimals can be processed at once */
    #ifndef _USE_FLOAT
    const size_t vec_capacity = sizeof(__m256d) / sizeof(decimal);
    #else
    const size_t vec_capacity = sizeof(__m256) / sizeof(decimal);
    #endif

    /* Prepare for parallelism */
    const auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    auto chunk_size = arr.size() / max_num_threads;
    /* Make sure that chunk_size is a multiple of 4 */
    chunk_size = chunk_size - chunk_size % vec_capacity;
    std::vector<decimal> sums(max_num_threads, 0);
    std::vector<decimal> sums_sq(max_num_threads, 0);

    /* Calculate the sums */
    #pragma omp parallel for default(none) shared(arr, sums, sums_sq, max_num_threads, chunk_size, vec_capacity)
    for (size_t i = 0; i < max_num_threads; i++) {
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
        decimal sum_arr[vec_capacity];
        decimal sum_sq_arr[vec_capacity];
        #ifndef _USE_FLOAT
        _mm256_storeu_pd(sum_arr, sum_vec);
        _mm256_storeu_pd(sum_sq_arr, sum_sq_vec);
        #else
        _mm256_storeu_ps(sum_arr, sum_vec);
        _mm256_storeu_ps(sum_sq_arr, sum_sq_vec);
        #endif

        /* Combine the sums */
        for (size_t j = 0; j < vec_capacity; j++) {
            sums[i] += sum_arr[j];
            sums_sq[i] += sum_sq_arr[j];
        }
    }

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
