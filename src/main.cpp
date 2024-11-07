#include <iostream>
#include <chrono>
#include "dataloader.h"
#include "merge_sort.h"
#include "statistics.h"

std::vector<double> gen_rand_arr(size_t size) {
    std::vector<double> arr(size);
    for (size_t i = 0; i < size; i++)
        arr[i] = rand() % 10000;
    return arr;
}

int main() {
    std::string filepath = "data/ACC_001.csv";
    patient_data data;

    std::cout << "Loading data from " << filepath << std::endl << std::endl;

    load_data(filepath, data);

    std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
    std::cout << data.x[0] << data.x[1] << data.x[2] << std::endl;
    std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
    std::cout << data.y[0] << data.y[1] << data.y[2] << std::endl;
    std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;
    std::cout << data.z[0] << data.z[1] << data.z[2] << std::endl;

    for (size_t i = 0; i < 3; i++) {
        std::vector<double> arr;
        if (i == 0)
            arr = data.x;
        else if (i == 1)
            arr = data.y;
        else
            arr = data.z;

        double sum = 0, sum_sq = 0;

        auto start = std::chrono::high_resolution_clock::now();

        my_merge_sort(arr, sum, sum_sq);

//        std::cout << std::is_sorted(arr.begin(), arr.end()) << std::endl;

        auto mad = compute_mad(arr);
        auto coef_var = compute_coef_var(sum, sum_sq, arr.size());

        auto end = std::chrono::high_resolution_clock::now();

        std::cout << "Mean absolute deviation: " << mad << std::endl;
        std::cout << "Coefficient of variation: " << coef_var << std::endl;

        std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;
    }

    return EXIT_SUCCESS;
}
