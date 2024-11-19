#include <iostream>
#include <chrono>
#include <variant>
#include <filesystem>

#include "utils/arg_parser.h"
#include "dataloader/dataloader.h"
#include "calculations/computations.h"

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code
 */
int main(int argc, char **argv) {
    /* Parse the arguments */
    arg_parser parser(argc, argv);
    parser.add_option(option("-f", "Filepath to the data file (required; mutually exclusive with -d)", true, false));
    parser.add_option(option("-d", "Filepath to the data directory (required; mutually exclusive with -f)", true, false));
    parser.add_option(option("--par", "Use parallel computation (serial by default)", false, false));
    parser.add_option(option("--vec", "Use vectorized computation (sequential by default)", false, false));
    parser.add_option(option("-h", "Print this help message", false, false));
    parser.add_option(option("--help", "Print this help message", false, false));
    auto args = parser.parse_args();

    /* Filepath to the data file(s) */
    const std::string dirpath = args.find("-d") != args.end() ? args["-d"] : "";
    const std::string filepath = args.find("-f") != args.end() ? args["-f"] : "";
    std::vector<std::string> files;
    const std::filesystem::path path = dirpath.empty() ? filepath : dirpath;
    if (std::filesystem::is_directory(path))
        for (const auto &entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file())
                files.push_back(entry.path().string());
        }
    else
        files.push_back(filepath);

    /* Policy for parallel and vectorized computation */
    std::string policy_p = "ser";
    std::string policy_v = "seq";
    if (args.find("--par") != args.end())
        policy_p = "par";
    if (args.find("--vec") != args.end())
        policy_v = "vec";

    /* Set the number of threads */
    policy_p == "ser" ? omp_set_num_threads(1) : omp_set_num_threads(omp_get_max_threads());

    /* Choose the policies for computations */
    std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy = std::execution::seq;
    std::variant<seq_comp, vec_comp> comp = seq_comp();
    if (policy_p == "par")
        policy = std::execution::par;
    if (policy_v == "vec")
        comp = vec_comp();

    std::cout << "Using " << (policy_p == "ser" ? "serial " : "parallel ")
              << (policy_v == "seq" ? "sequential " : "vectorized ") << "computation..." << std::endl;
    std::cout << "Using " << (sizeof(decimal)) << "-byte floating point numbers..." << std::endl << std::endl;

    /* Iterate over the files if the path is a directory */
    for (auto &file : files) {
        /* Load the data */
        patient_data data;
        std::cout << "Loading data from " << file << "..." << std::endl;

        auto start = std::chrono::high_resolution_clock::now();  /* Time measurement */

        std::visit([&](auto &&exec) {
            load_data_parallel(exec, file, data);
        }, policy);

        auto end = std::chrono::high_resolution_clock::now();  /* Time measurement */
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Loaded in " << elapsed << "ms" << std::endl << std::endl;

        /* Print the number of loaded data (for checking) */
        std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
        std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
        std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;

        /* Compute the mean absolute deviation and coefficient of variation for X, Y and Z respectively */
        std::vector<std::vector<decimal>> vectors = {data.x, data.y, data.z};
        std::vector<std::string> labels = {"X data:", "Y data:", "Z data:"};
        /* For each data vector */
        for (size_t i = 0; i < vectors.size(); i++) {
            std::cout << labels[i] << std::endl;

            start = std::chrono::high_resolution_clock::now();  /* Time measurement */

            /*
             * Actual computation -- uses the variant and the visitor pattern
             * (mimics dynamic polymorphism, but with no runtime overhead)
             */
            auto mad = std::visit([&](auto &&comp) -> decimal {
                return std::visit([&](auto &&exec) -> decimal {
                    return comp.compute_mad(exec, vectors[i]);
                }, policy);
            }, comp);
            auto coef_var = std::visit([&](auto &&comp) -> decimal {
                return std::visit([&](auto &&exec) -> decimal {
                    return comp.compute_coef_var(exec, vectors[i]);
                }, policy);
            }, comp);

            end = std::chrono::high_resolution_clock::now();  /* Time measurement */
            elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout << "Mean absolute deviation: " << mad << std::endl;
            std::cout << "Coefficient of variation: " << coef_var << std::endl;

            std::cout << "Time taken: " << elapsed << "ms" << std::endl << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
