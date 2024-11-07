#include <iostream>
#include <chrono>
#include "merge_sort.h"
#include "statistics.h"

std::vector<double> gen_rand_arr(size_t size) {
    std::vector<double> arr(size);
    for (size_t i = 0; i < size; i++)
        arr[i] = rand() % 10000;
    return arr;
}

int main() {
    auto arr = gen_rand_arr(1000000);

    double sum = 0, sum_sq = 0;

    auto start = std::chrono::high_resolution_clock::now();
    my_merge_sort(arr, sum, sum_sq);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    std::cout << std::is_sorted(arr.begin(), arr.end()) << std::endl;

    auto mad = compute_mad(arr);
    auto coef_var = compute_coef_var(sum, sum_sq, arr.size());

    std::cout << "Mean absolute deviation: " << mad << std::endl;
    std::cout << "Coefficient of variation: " << coef_var << std::endl;

    return 0;
}
