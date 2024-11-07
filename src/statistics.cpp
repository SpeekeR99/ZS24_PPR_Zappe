#include "statistics.h"
#include "merge_sort.h"

double compute_mad(const std::vector<double>& arr) {
    auto median = arr[arr.size() / 2];

    std::vector<double> diff(arr.size());
    for (size_t i = 0; i < arr.size(); i++)
        diff[i] = std::abs(arr[i] - median);

    std::sort(diff.begin(), diff.end()); /* TODO: Change this to bitonic merge */

    return diff[diff.size() / 2];
}

double compute_coef_var(double sum, double sum_sq, size_t n) {
    return std::sqrt((sum_sq - sum * sum / n) / n) / (sum / n);
}
