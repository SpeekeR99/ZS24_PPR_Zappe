#include "statistics.h"

void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq) {
    sum = 0;
    sum_sq = 0;

    /* Prepare for parallelism */
    auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    auto chunk_size = arr.size() / max_num_threads;
    std::vector<double> sums(max_num_threads, 0);
    std::vector<double> sums_sq(max_num_threads, 0);

    /* Calculate the sums */
    #pragma omp parallel for default(none) shared(arr, sums, sums_sq, max_num_threads, chunk_size)
    for (size_t i = 0; i < max_num_threads; i++) {
        size_t start = i * chunk_size;
        /* Final thread may have to handle a little more elements */
        size_t end = (i == max_num_threads - 1) ? arr.size() : (i + 1) * chunk_size;

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

double compute_mad(std::vector<double> &arr) {
    /* Sort the array for median calculation */
    merge_sort(arr);

    std::cout << (std::is_sorted(arr.begin(), arr.end()) ? "Sorted" : "Not sorted") << std::endl;

    /* Get the median */
    auto median = (arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0;

    /* Calculate the absolute differences from the median */
    std::vector<double> diff(arr.size());
    #pragma omp parallel for default(none) shared(arr, diff, median)
    for (size_t i = 0; i < arr.size(); i++)
        diff[i] = std::abs(arr[i] - median);

    /*
     * Since the array is "half-sorted", we can abuse it:
     * We start at the center, where we had the original median
     * We then move outwards, comparing elements from the left and right halves
     * We always pick the smaller of the two, move the corresponding pointer until we reach the new median
     */
    size_t left = diff.size() / 2 - 1, right = diff.size() / 2;
    double prev = 0, curr = 0;
    for (size_t i = 0; i <= diff.size() / 2; i++) {
        prev = curr;
        curr = diff[left] >= diff[right] ? diff[right++] : diff[left--];
    }

    return (arr.size() & 1) ? curr : (prev + curr) / 2.0;
}

double compute_coef_var(const std::vector<double> &arr) {
    /* Using the formula: sqrt((sum of squares - sum^2 / n) / n) / (sum / n) */
    size_t n = arr.size();

    double sum = 0, sum_sq = 0;
    compute_sums(arr, sum, sum_sq);

    return std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n);
}
