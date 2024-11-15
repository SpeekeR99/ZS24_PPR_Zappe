#include "statistics.h"

void compute_sums(const std::vector<double> &arr, double &sum, double &sum_sq) {
    sum = 0;
    sum_sq = 0;
    auto max_num_threads = static_cast<size_t>(omp_get_max_threads());
    auto chunk_size = arr.size() / max_num_threads;
    std::vector<double> sums(max_num_threads, 0);
    std::vector<double> sums_sq(max_num_threads, 0);

    #pragma omp parallel for default(none) shared(arr, sums, sums_sq, max_num_threads, chunk_size)
    for (size_t i = 0; i < max_num_threads; i++) {
        size_t start = i * chunk_size;
        size_t end = (i == max_num_threads - 1) ? arr.size() : (i + 1) * chunk_size;

        for (size_t j = start; j < end; j++) {
            sums[i] += arr[j];
            sums_sq[i] += arr[j] * arr[j];
        }
    }

    for (size_t i = 0; i < max_num_threads; i++) {
        sum += sums[i];
        sum_sq += sums_sq[i];
    }
}

double compute_mad(std::vector<double> &arr) {
    my_merge_sort(arr);

    std::cout << (std::is_sorted(arr.begin(), arr.end()) ? "Sorted" : "Not sorted") << std::endl;

    auto median = (arr[arr.size() / 2] + arr[(arr.size() - 1) / 2]) / 2.0;

    std::vector<double> diff(arr.size());
    #pragma omp parallel for default(none) shared(arr, diff, median)
    for (size_t i = 0; i < arr.size(); i++)
        diff[i] = std::abs(arr[i] - median);

    /* Since the array is "half-sorted", we can abuse it */
    size_t left = diff.size() / 2 - 1, right = diff.size() / 2;
    double prev = 0, curr = 0;
    for (size_t i = 0; i <= diff.size() / 2; i++) {
        prev = curr;
        curr = diff[left] >= diff[right] ? diff[right++] : diff[left--];
    }

    return (arr.size() & 1) ? curr : (prev + curr) / 2.0;
}

double compute_coef_var(const std::vector<double> &arr) {
    size_t n = arr.size();

    double sum = 0, sum_sq = 0;
    compute_sums(arr, sum, sum_sq);

    return std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n);
}
