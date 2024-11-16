#include <iostream>
#include <chrono>
#include "arg_parser.h"
#include "dataloader.h"
#include "computations.h"

int main(int argc, char **argv) {
    /* Parse the arguments */
    arg_parser parser(argc, argv);
    parser.add_option(option("-f", "Filepath to the data file", true, true));
    parser.add_option(option("-h", "Print this help message", false, false));
    parser.add_option(option("--help", "Print this help message", false, false));
    parser.add_option(option("--par", "Use parallel computation (serial by default)", false, false));
    parser.add_option(option("--vec", "Use vectorized computation (sequential by default)", false, false));
    auto args = parser.parse_args();

    /* Filepath to the data file */
    const std::string filepath = args["-f"];

    /* Policy for parallel and vectorized computation */
    std::string policy_p = "ser";
    std::string policy_v = "seq";
    if (args.find("--par") != args.end())
        policy_p = "par";
    if (args.find("--vec") != args.end())
        policy_v = "vec";

    /* Set the number of threads */
    policy_p == "ser" ? omp_set_num_threads(1) : omp_set_num_threads(omp_get_max_threads());

    /* Choose the policy for computations */
    seq_comp seq{};
    vec_comp vec{};
    computations &comp = policy_v == "seq" ? reinterpret_cast<computations&>(seq) : reinterpret_cast<computations&>(vec);
    std::cout << "Using " << (policy_p == "ser" ? "serial " : "parallel ") << (policy_v == "seq" ? "sequential " : "vectorized ") << "computation..." << std::endl << std::endl;

    /* Load the data */
    patient_data data;
    std::cout << "Loading data from " << filepath << "..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();  /* Time measurement */

    load_data_parallel(filepath, data);

    auto end = std::chrono::high_resolution_clock::now();  /* Time measurement */
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Loaded in " << elapsed << "ms" << std::endl << std::endl;

    /* Print the number of loaded data (for checking) */
    std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
    std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
    std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;

    /* Compute the mean absolute deviation and coefficient of variation for X, Y and Z respectively */
    std::vector<std::vector<double>> vectors = {data.x, data.y, data.z};
    std::vector<std::string> labels = {"X data:", "Y data:", "Z data:"};
    /* For each data vector */
    for (size_t i = 0; i < vectors.size(); i++) {
        std::cout << labels[i] << std::endl;

        start = std::chrono::high_resolution_clock::now();  /* Time measurement */

        /* Actual computation */
        auto mad = comp.compute_mad(vectors[i]);
        auto coef_var = comp.compute_coef_var(vectors[i]);

        end = std::chrono::high_resolution_clock::now();  /* Time measurement */
        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Mean absolute deviation: " << mad << std::endl;
        std::cout << "Coefficient of variation: " << coef_var << std::endl;

        std::cout << "Time taken: " << elapsed << "ms" << std::endl << std::endl;
    }

    return EXIT_SUCCESS;
}
