#include <iostream>
#include <chrono>
#include "dataloader.h"
#include "statistics.h"

int main() {
    std::string filepath = "data/ACC_001.csv";
    patient_data data;

    std::cout << "Loading data from " << filepath << "..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    load_data_super_fast(filepath, data);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Loaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
    std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
    std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;

    auto policy = std::execution::seq;

    for (size_t i = 0; i < 3; i++) {
        std::vector<double> *arr;
        if (i == 0) {
            std::cout << "X data:" << std::endl;
            arr = &data.x;
        } else if (i == 1) {
            std::cout << "Y data:" << std::endl;
            arr = &data.y;
        } else {
            std::cout << "Z data:" << std::endl;
            arr = &data.z;
        }

        start = std::chrono::high_resolution_clock::now();

        auto mad = compute_mad(*arr, policy);
        auto coef_var = compute_coef_var(*arr, policy);

        end = std::chrono::high_resolution_clock::now();

        std::cout << "Mean absolute deviation: " << mad << std::endl;
        std::cout << "Coefficient of variation: " << coef_var << std::endl;

        std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;
    }

    return EXIT_SUCCESS;
}
