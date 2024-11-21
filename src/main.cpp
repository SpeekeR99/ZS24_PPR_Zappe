#include <iostream>
#include <chrono>
#include <variant>
#include <filesystem>

#include "utils/arg_parser.h"
#include "dataloader/dataloader.h"
#include "calculations/cpu/computations.h"
#include "calculations/gpu/gpu.h"
#include "my_drawing//svg_generator.h"

/**
 * Parses the arguments using the arg_parser class
 * @param argc Argument count
 * @param argv Argument values
 * @return A map of options and parsed values
 */
std::map<std::string, std::string> parse_args(int argc, char **argv) {
    /* Parse the arguments */
    arg_parser parser(argc, argv);
    parser.add_option(option("-f", "Filepath to the data file (mutually exclusive with -d)", true, true));
    parser.add_option(option("-d", "Filepath to the data directory (mutually exclusive with -f)", true, true));
    parser.add_option(option("-r", "Number of repetitions -- each computation will be repeated n number of times and median is printed out (default: 1)", true, false));
    parser.add_option(option("-n", "Number of chunks to split the data into (granularity for graphs) (default: 1)", true, false));
    parser.add_option(option("--par", "Use parallel computation (serial by default)", false, false));
    parser.add_option(option("--vec", "Use vectorized computation (sequential by default)", false, false));
    parser.add_option(option("--all", "Use all available policies combinations (used for graphs)", false, false));
    parser.add_option(option("-h", "Print this help message", false, false));
    parser.add_option(option("--help", "Print this help message", false, false));

    return parser.parse_args();
}

/**
 * Get the files from the directory or the file itself
 * -f flag and -d flag are mutually exclusive, only one can be used by the user (checked in the arg_parser)
 * @param args Arguments
 * @return A vector of file paths (strings)
 */
std::vector<std::string> get_files(std::map<std::string, std::string> &args) {
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

    return files;
}

/**
 * Choose the policies for computations
 * @param args Arguments
 * @param all Whether all policy combinations should be used (--all flag) (used here only for print)
 *            (because --par --vec and --all are not mutually exclusive, so user can use them all)
 *            (if --all is used with --par and/or --vec flags, they will be ignored, --all has priority)
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 */
void choose_policies(std::map<std::string, std::string> &args, bool all, std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> &policy, std::variant<seq_comp, vec_comp> &comp) {
    /* Policy for parallel and vectorized computation */
    std::string policy_p = "ser";
    std::string policy_v = "seq";
    if (args.find("--par") != args.end())
        policy_p = "par";
    if (args.find("--vec") != args.end())
        policy_v = "vec";

    if (all)
        policy_p = "par";  /* For the parallel data load */
    else
        std::cout << "Using " << (policy_p == "ser" ? "serial " : "parallel ")
                  << (policy_v == "seq" ? "sequential " : "vectorized ") << "computation..." << std::endl;

    /* Choose execution policy */
    policy = std::execution::seq;
    if (policy_p == "par")
        policy = std::execution::par;

    /* Choose computation */
    comp = seq_comp();
    if (policy_v == "vec")
        comp = vec_comp();
}

/**
 * Execute the computations for the given repetitions
 * This is made into function for easy handling of the --all flag (also for better readability)
 * @param data Data to be used for computations
 * @param num_data_points Number of data points to be used for computation (deep copy of the data param)
 * @param repetitions Number of repetitions (computation is repeated n number of times -> median of the measurements)
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 */
void execute_computations_for_repetitions(patient_data &data, size_t num_data_points, size_t repetitions, std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy, std::variant<seq_comp, vec_comp> comp) {
    /* For each repetition -- purpose for median of the measured times (3 hard coded as X, Y, Z) */
    std::vector<std::vector<long long int>> measured_times(3, std::vector<long long int>(repetitions));
    std::vector<std::vector<decimal>> mads(3, std::vector<decimal>(repetitions));
    std::vector<std::vector<decimal>> coef_vars(3, std::vector<decimal>(repetitions));

    for (size_t i = 0; i < repetitions; i++) {
        std::cout << "Repetition " << i + 1 << "..." << std::endl;

        /* Create deep copies of the data */
        std::vector<decimal> copy_x(data.x.begin(), data.x.begin() + static_cast<int>(num_data_points));
        std::vector<decimal> copy_y(data.y.begin(), data.y.begin() + static_cast<int>(num_data_points));
        std::vector<decimal> copy_z(data.z.begin(), data.z.begin() + static_cast<int>(num_data_points));

        /* Compute the mean absolute deviation and coefficient of variation for X, Y and Z respectively */
        std::vector<std::vector<decimal>> vectors = {copy_x, copy_y, copy_z};
        /* For each data vector */
        for (size_t j = 0; j < vectors.size(); j++) {
            auto start = std::chrono::high_resolution_clock::now();  /* Time measurement */

            /*
             * Actual computation -- uses the variant and the visitor pattern
             * (mimics dynamic polymorphism, but with no runtime overhead)
             */
            auto mad = std::visit([&](auto &&comp) -> decimal {
                return std::visit([&](auto &&exec) -> decimal {
                    return comp.compute_mad(exec, vectors[j]);
                }, policy);
            }, comp);
            auto coef_var = std::visit([&](auto &&comp) -> decimal {
                return std::visit([&](auto &&exec) -> decimal {
                    return comp.compute_coef_var(exec, vectors[j]);
                }, policy);
            }, comp);

            auto end = std::chrono::high_resolution_clock::now();  /* Time measurement */
            auto computed_in = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            /* Store the measured times for median */
            measured_times[j][i] = computed_in;
            mads[j][i] = mad;
            coef_vars[j][i] = coef_var;
        }
    }

    /* Print the median (and mean) of the measured times */
    std::vector<std::string> labels = {"X", "Y", "Z"};
    for (size_t i = 0; i < labels.size(); i++) {
        std::cout << "For " << labels[i] << " data:" << std::endl;

        /* Pick the median */
        std::sort(std::execution::par_unseq, measured_times[i].begin(), measured_times[i].end());
        std::sort(std::execution::par_unseq, mads[i].begin(), mads[i].end());
        std::sort(std::execution::par_unseq, coef_vars[i].begin(), coef_vars[i].end());
        const auto computed_in_med = static_cast<double>(measured_times[i][measured_times[i].size() / 2] + measured_times[i][(measured_times[i].size() - 1) / 2]) / 2.0;
        const auto mad_med = (mads[i][mads[i].size() / 2] + mads[i][(mads[i].size() - 1) / 2]) / 2.0;
        const auto coef_var_med = (coef_vars[i][coef_vars[i].size() / 2] + coef_vars[i][(coef_vars[i].size() - 1) / 2]) / 2.0;

        std::cout << "Medians:" << std::endl;
        std::cout << "Mean absolute deviation: " << mad_med << std::endl;
        std::cout << "Coefficient of variation: " << coef_var_med << std::endl;
        std::cout << "Time taken " << computed_in_med << "ms" << std::endl;
    }
}

/**
 * Execute the computations
 * @param files Files to be processed
 * @param repetitions Repetitions for each computation
 * @param num_chunks Number of chunks to split the data into
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 * @param all Whether all policy combinations should be used (--all flag)
 */
void execute_computations(std::vector<std::string> files, size_t repetitions, size_t num_chunks, std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy, std::variant<seq_comp, vec_comp> comp, bool all) {
    /* For each file we are processing */
    for (auto &file : files) {
        /* Load the data */
        patient_data data;

        std::cout << "Loading data from " << file << "..." << std::endl;
        auto start = std::chrono::high_resolution_clock::now();  /* Time measurement */

        std::visit([&](auto &&exec) {
            load_data_parallel(exec, file, data);
        }, policy);

        auto end = std::chrono::high_resolution_clock::now();  /* Time measurement */
        auto loaded_in = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Loaded in " << loaded_in << "ms" << std::endl << std::endl;

        /* Print the number of loaded data (for checking) */
        std::cout << "Loaded " << data.x.size() << " X data" << std::endl;
        std::cout << "Loaded " << data.y.size() << " Y data" << std::endl;
        std::cout << "Loaded " << data.z.size() << " Z data" << std::endl << std::endl;

        /* For each split chunk of the loaded data */
        for (size_t i = 0; i < num_chunks; i++) {
            const auto num_data_points = data.x.size() / num_chunks * (i + 1);
            std::cout << "Using " << num_data_points << " data points for computation..." << std::endl << std::endl;

            /* If we are using all combinations of policies -- one more "for loop" before the repetitions */
            if (all) {
                /* For each policy combination */
                std::cout << "Using all policy combinations..." << std::endl;
                std::cout << "Serial sequential computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::seq, seq_comp());
                std::cout << "Serial vectorized computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::seq, vec_comp());
                std::cout << "Parallel sequential computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::par, seq_comp());
                std::cout << "Parallel vectorized computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::par, vec_comp());
            } else {  /* If only one policy is used, go straight to repetitions */
                execute_computations_for_repetitions(data, num_data_points, repetitions, policy, comp);
            }
        }
    }
}

/**
 * Main function
 * @param argc Argument count
 * @param argv Argument values
 * @return Exit code
 */
int main(int argc, char **argv) {
//    /* Parse the arguments */
//    auto args = parse_args(argc, argv);
//
//    /* User arguments were valid, let him know about single / double precision */
//    std::cout << "Using " << (sizeof(decimal)) << "-byte floating point numbers..." << std::endl << std::endl;
//
//    /* Filepath to the data file(s) */
//    auto files = get_files(args);
//
//    /* Number of repetitions and chunks */
//    const size_t repetitions = args.find("-r") != args.end() ? std::stoi(args["-r"]) : 1;
//    const size_t num_chunks = args.find("-n") != args.end() ? std::stoi(args["-n"]) : 1;
//
//    /* Choose the policies for computations */
//    std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy;
//    std::variant<seq_comp, vec_comp> comp;
//    bool all = args.find("--all") != args.end();
//    choose_policies(args, all, policy, comp);
//
//    /* Prepare res directory for the plots, if it does not exist */
//    if (!std::filesystem::exists("res"))
//        std::filesystem::create_directory("res");
//
//    /*
//     * Execute the computations:
//     * For each file, load the data
//     * For each split chunk of the data (purpose: graphs -- lines X points)
//     * Now branching: if all is true, for each policy combination (purpose: graphs -- more lines)
//     *                else for the chosen policy combination by the user
//     * For each repetition, (deep) copy the data (purpose: median of the measured times)
//     * For each vector X, Y, Z from the data, finally compute the MAD and CV
//     */
//    execute_computations(files, repetitions, num_chunks, policy, comp, all);
//
//    /* Example plot for my future self */
//    std::vector<double> x = {0, 1, 2, 3, 4, 5};
//    std::vector<double> y1 = {0, 1, 4, 9, 16, 25};
//    std::vector<double> y2 = {0, 1, 2, 3, 4, 5};
//
//    std::vector<std::vector<double>> x_values_list = {x, x};
//    std::vector<std::vector<double>> y_values_list = {y1, y2};
//
//    std::vector<std::string> labels = {"y1", "y2"};
//
//    plot_line_chart("res/test.svg", x_values_list, y_values_list, "Test Chart", "X", "Y", labels);

    gpu_manager gpu;

    // Prepare data for the test
    const unsigned int n = 1024; // Number of elements
    std::vector<float> A(n, 1.0f); // Vector A filled with 1.0
    std::vector<float> B(n, 2.0f); // Vector B filled with 2.0
    std::vector<float> C(n);       // Result vector

    const std::string kernel_source = R"(__kernel void vector_add(__global const float* A, __global const float* B, __global float* C, const unsigned int n) {
        int id = get_global_id(0);
        if (id < n) {
            C[id] = A[id] + B[id];
        }
    })";

    // Create program and build it
    cl::Program program(gpu.context, kernel_source);
    try {
        program.build({ gpu.device });
    } catch (const std::exception &e) {
        // If build fails, print the log
        std::cerr << "Build Log:\n"
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(gpu.device)
                  << std::endl;
        throw;
    }

    // Create kernel
    cl::Kernel kernel(program, "vector_add");

    // Create buffers
    cl::Buffer buffer_A(gpu.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, n * sizeof(float), A.data());
    cl::Buffer buffer_B(gpu.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, n * sizeof(float), B.data());
    cl::Buffer buffer_C(gpu.context, CL_MEM_WRITE_ONLY, n * sizeof(float));

    // Set kernel arguments
    kernel.setArg(0, buffer_A);
    kernel.setArg(1, buffer_B);
    kernel.setArg(2, buffer_C);
    kernel.setArg(3, n);

    // Execute kernel
    cl::NDRange global_work_size(n);
    gpu.queue.enqueueNDRangeKernel(kernel, cl::NullRange, global_work_size);

    // Read results
    gpu.queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, n * sizeof(float), C.data());

    // Print results
    for (size_t i = 0; i < n; i++)
        std::cout << C[i] << " ";

    return EXIT_SUCCESS;
}
