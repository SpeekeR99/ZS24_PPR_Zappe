#include <iostream>
#include <chrono>
#include "arg_parser.h"
#include "dataloader.h"
#include "computations.h"

int main(int argc, char **argv) {
    arg_parser parser(argc, argv);
    parser.add_option(option("-f", "Filepath to the data file", true, true));
    parser.add_option(option("-h", "Print this help message", false, false));
    parser.add_option(option("--help", "Print this help message", false, false));
    parser.add_option(option("--serial", "Use serial computation", false, false));
    parser.add_option(option("--parallel", "Use parallel computation", false, false));
    parser.add_option(option("--sequential", "Use sequential computation", false, false));
    parser.add_option(option("--vectorized", "Use vectorized computation", false, false));

    auto args = parser.parse_args();

    const std::string filepath = args["-f"];

    std::string policy_p = "ser";
    std::string policy_v = "seq";
    if (args.find("--parallel") != args.end())
        policy_p = "par";
    if (args.find("--vectorized") != args.end())
        policy_v = "vec";

    policy_p == "ser" ? omp_set_num_threads(1) : omp_set_num_threads(omp_get_max_threads());
    seq_comp seq{};
    vec_comp vec{};
    computations &comp = policy_v == "seq" ? reinterpret_cast<computations&>(seq) : reinterpret_cast<computations&>(vec);

    patient_data data;

    std::cout << "Using " << (policy_p == "ser" ? "serial " : "parallel ") << (policy_v == "seq" ? "sequential " : "vectorized ") << "computation..." << std::endl << std::endl;
    std::cout << "Loading data from " << filepath << "..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    load_data_parallel(filepath, data);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Loaded in " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;

    std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
    std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
    std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;

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

        auto mad = comp.compute_mad(*arr);
        auto coef_var = comp.compute_coef_var(*arr);

        end = std::chrono::high_resolution_clock::now();

        std::cout << "Mean absolute deviation: " << mad << std::endl;
        std::cout << "Coefficient of variation: " << coef_var << std::endl;

        std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl << std::endl;
    }

    return EXIT_SUCCESS;
}
