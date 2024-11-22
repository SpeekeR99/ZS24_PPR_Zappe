#include <iostream>
#include <chrono>
#include <variant>
#include <filesystem>

#include "utils/arg_parser.h"
#include "dataloader/dataloader.h"
#include "calculations/cpu/cpu_comps.h"
#include "calculations/gpu/gpu_comps.h"
#include "my_drawing/svg_generator.h"

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
    parser.add_option(option("-n", "Number of batches to split the data into (granularity for graphs) (default: 1)", true, false));
    parser.add_option(option("--par", "Use parallel computation (serial by default)", false, false));
    parser.add_option(option("--vec", "Use vectorized computation (sequential by default)", false, false));
    parser.add_option(option("--gpu", "Use GPU computation (CPU by default)", false, false));
    parser.add_option(option("--all", "Use all available policies combinations (used for graphs)", false, false));
    parser.add_option(option("--no_graphs", "Do not plot the results (default: plot the results)", false, false));
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
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 * @param gpu Whether GPU computation should be used (--gpu flag)
 * @param all Whether all policy combinations should be used (--all flag) (used here only for print)
 *            (because --par --vec and --all are not mutually exclusive, so user can use them all)
 *            (if --all is used with --par and/or --vec flags, they will be ignored, --all has priority)
 */
void choose_policies(
    std::map<std::string, std::string> &args,
    std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> &policy,
    std::variant<seq_comp, vec_comp, gpu_comps> &comp,
    bool gpu,
    bool all
) {
    /* Policy for parallel and vectorized computation */
    std::string policy_p = "ser";
    std::string policy_v = "seq";
    if (args.find("--par") != args.end())
        policy_p = "par";
    if (args.find("--vec") != args.end())
        policy_v = "vec";

    if (all)
        policy_p = "par";  /* For the parallel data load */
    else if (!gpu)
        std::cout << "Using " << (policy_p == "ser" ? "serial " : "parallel ")
                  << (policy_v == "seq" ? "sequential " : "vectorized ") << "computation..." << std::endl;
    else
        std::cout << "Using GPU computation..." << std::endl;

    /* Choose execution policy */
    policy = std::execution::seq;
    if (policy_p == "par")
        policy = std::execution::par;

    /* Choose computation */
    comp = seq_comp();
    if (policy_v == "vec")
        comp = vec_comp();
    if (gpu)
        comp = gpu_comps();
}

/**
 * Execute the computations for the given repetitions
 * This is made into function for easy handling of the --all flag (also for better readability)
 * @param data Data to be used for computations
 * @param num_data_points Number of data points to be used for computation (deep copy of the data param)
 * @param repetitions Number of repetitions (computation is repeated n number of times -> median of the measurements)
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 * @param results Vector to store the results for later plotting
 */
void execute_computations_for_repetitions(
    patient_data &data,
    const size_t num_data_points,
    const size_t repetitions,
    std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy,
    std::variant<seq_comp, vec_comp, gpu_comps> comp,
    std::vector<double> &results
) {
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

        std::cout << "Mean absolute deviation: " << mad_med << std::endl;
        std::cout << "Coefficient of variation: " << coef_var_med << std::endl;
        std::cout << "Time taken " << computed_in_med << "ms" << std::endl;

        /* Store the medians for later plotting */
        results.emplace_back(mad_med);
        results.emplace_back(coef_var_med);
        results.emplace_back(computed_in_med);
    }
}

/**
 * Plots the results based on measured times, MADs and CVs
 * If --all flag is used, the results are plotted separately for each computation method
 * else the results are plotted for X, Y, Z separately
 * @param results Results to be plotted (Y axis)
 * @param batches Batches for the X axis
 * @param all All policy combinations used (--all flag)
 */
void plot_results(const std::vector<double> &results, const std::vector<double> &batches, bool all) {
    std::cout << "Plotting the results..." << std::endl;

    /* Plot the results */
    if (all) {
        /* 5 results for each data point (SerSeq, SerVec, ParSeq, ParVec, GPU) */
        const std::vector<std::string> labels = {"SerSeq", "SerVec", "ParSeq", "ParVec", "GPU"};
        /* 3 results per data point (X, Y, Z) */
        const std::vector<std::string> sub_labels = {"X", "Y", "Z"};

        /* Extract the results for each data point */
        std::vector<std::vector<std::vector<double>>> mads(labels.size(), std::vector<std::vector<double>>(sub_labels.size()));
        std::vector<std::vector<std::vector<double>>> cvs(labels.size(), std::vector<std::vector<double>>(sub_labels.size()));
        std::vector<std::vector<std::vector<double>>> times(labels.size(), std::vector<std::vector<double>>(sub_labels.size()));
        for (size_t i = 0, j = 0; i < results.size(); i += sub_labels.size() * 3, j = ++j % labels.size()) {
            for (size_t k = 0; k < sub_labels.size(); k++) {
                mads[j][k].emplace_back(results[i + k * 3]);
                cvs[j][k].emplace_back(results[i + k * 3 + 1]);
                times[j][k].emplace_back(results[i + k * 3 + 2]);
            }
        }

        /* Create names for the plots */
        const auto now = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());

        /* Plot the results separately for X, Y, Z */
        for (size_t i = 0; i < sub_labels.size(); i++) {
            /* Create the names for real now */
            const auto name_times = "res/times_" + sub_labels[i] + "_" + now + ".svg";
            const auto name_mads = "res/mads_" + sub_labels[i] + "_" + now + ".svg";
            const auto name_cvs = "res/cvs_" + sub_labels[i] + "_" + now + ".svg";

            /* Plot the results */
            auto title = "Time taken for computation (" + sub_labels[i] + ")";
            std::vector<std::vector<double>> x_values_list = {batches, batches, batches, batches, batches};
            std::vector<std::vector<double>> y_values_list = {times[0][i], times[1][i], times[2][i], times[3][i], times[4][i]};
            plot_line_chart(name_times, x_values_list, y_values_list, title, "Data batch size", "Time (ms)", labels);

            title = "Mean Absolute Deviation (" + sub_labels[i] + ")";
            y_values_list = {mads[0][i], mads[1][i], mads[2][i], mads[3][i], mads[4][i]};
            plot_line_chart(name_mads, x_values_list, y_values_list, title, "Data batch size", "MAD", labels);

            title = "Coefficient of Variation (" + sub_labels[i] + ")";
            y_values_list = {cvs[0][i], cvs[1][i], cvs[2][i], cvs[3][i], cvs[4][i]};
            plot_line_chart(name_cvs, x_values_list, y_values_list, title, "Data batch size", "CV", labels);
        }
        /* If only one policy is used, plot the results for X, Y, Z instead */
    } else {
        /* 3 results for each data point (X, Y, Z) */
        const std::vector<std::string> labels = {"X", "Y", "Z"};

        /* Extract the results for each data point */
        std::vector<std::vector<double>> mads(labels.size());
        std::vector<std::vector<double>> cvs(labels.size());
        std::vector<std::vector<double>> times(labels.size());
        for (size_t i = 0, j = 0; i < results.size(); i += labels.size(), j = ++j % labels.size()) {
            mads[j].emplace_back(results[i]);
            cvs[j].emplace_back(results[i + 1]);
            times[j].emplace_back(results[i + 2]);
        }

        /* Create names for the plots */
        const auto now = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        const std::string name_times = "res/times_" + now + ".svg";
        const std::string name_mads = "res/mads_" + now + ".svg";
        const std::string name_cvs = "res/cvs_" + now + ".svg";

        /* Plot the results */
        std::vector<std::vector<double>> x_values_list = {batches, batches, batches};
        std::vector<std::vector<double>> y_values_list = {times[0], times[1], times[2]};
        plot_line_chart(name_times, x_values_list, y_values_list, "Time taken for computation", "Data batch size", "Time (ms)", labels);

        y_values_list = {mads[0], mads[1], mads[2]};
        plot_line_chart(name_mads, x_values_list, y_values_list, "Mean Absolute Deviation", "Data batch size", "MAD", labels);

        y_values_list = {cvs[0], cvs[1], cvs[2]};
        plot_line_chart(name_cvs, x_values_list, y_values_list, "Coefficient of Variation", "Data batch size", "CV", labels);
    }

    std::cout << "You can find the plots in the res directory." << std::endl;
}

/**
 * Execute the computations
 * @param files Files to be processed
 * @param repetitions Repetitions for each computation
 * @param num_batches Number of batches to split the data into
 * @param policy Policy for parallel and vectorized computation
 * @param comp Computation (sequential or vectorized)
 * @param all Whether all policy combinations should be used (--all flag)
 * @param results Results to be plotted (Y axis)
 * @param batches Batches for the X axis
 */
void execute_computations(
    const std::vector<std::string> &files,
    const size_t repetitions,
    const size_t num_batches,
    const std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> &policy,
    const std::variant<seq_comp, vec_comp, gpu_comps> &comp,
    bool all,
    std::vector<double> &results,
    std::vector<double> &batches
) {
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

        /* For each split chunk (batch) of the loaded data */
        for (size_t i = 0; i < num_batches; i++) {
            const auto num_data_points = data.x.size() / num_batches * (i + 1);
            batches.emplace_back(num_data_points);
            std::cout << "Using " << num_data_points << " data points for computation..." << std::endl << std::endl;

            /* If we are using all combinations of policies -- one more "for loop" before the repetitions */
            if (all) {
                /* For each policy combination */
                std::cout << "Using all policy combinations..." << std::endl;
                std::cout << "Serial sequential computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::seq, seq_comp(), results);
                std::cout << "Serial vectorized computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::seq, vec_comp(), results);
                std::cout << "Parallel sequential computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::par, seq_comp(), results);
                std::cout << "Parallel vectorized computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::par, vec_comp(), results);
                std::cout << "GPU computation..." << std::endl;
                execute_computations_for_repetitions(data, num_data_points, repetitions, std::execution::seq, gpu_comps(), results);
            } else {  /* If only one policy is used, go straight to repetitions */
                execute_computations_for_repetitions(data, num_data_points, repetitions, policy, comp, results);
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
    /* Parse the arguments */
    auto args = parse_args(argc, argv);

    /* User arguments were valid, let him know about single / double precision */
    std::cout << "Using " << (sizeof(decimal)) << "-byte floating point numbers..." << std::endl << std::endl;

    /* Filepath to the data file(s) */
    auto files = get_files(args);

    /* Number of repetitions and batches (or chunks) */
    const size_t repetitions = args.find("-r") != args.end() ? std::stoi(args["-r"]) : 1;
    const size_t num_batches = args.find("-n") != args.end() ? std::stoi(args["-n"]) : 1;

    /* Choose the policies for computations */
    std::variant<std::execution::sequenced_policy, std::execution::parallel_policy> policy;
    std::variant<seq_comp, vec_comp, gpu_comps> comp;
    bool gpu = args.find("--gpu") != args.end();
    bool all = args.find("--all") != args.end();
    choose_policies(args, policy, comp, gpu, all);

    /* Prepare res directory for the plots, if it does not exist */
    if (!std::filesystem::exists("res"))
        std::filesystem::create_directory("res");

    /* Prepare structures to save the results for later plotting */
    std::vector<double> results;
    std::vector<double> batches;  /* This should correctly be size_t, not double, but for plotting purposes -- double */

    /*
     * Execute the computations:
     * For each file, load the data
     * For each batch (or split chunk) of the data (purpose: graphs -- lines X points)
     * Now branching: if all is true, for each policy combination (purpose: graphs -- more lines)
     *                else for the chosen policy combination by the user
     * For each repetition, (deep) copy the data (purpose: median of the measured times)
     * For each vector X, Y, Z from the data, finally compute the MAD and CV
     */
    execute_computations(files, repetitions, num_batches, policy, comp, all, results, batches);

    /* Plot the results (if the user did not specify --no_graphs flag) */
    if (args.find("--no_graphs") == args.end())
        plot_results(results, batches, all);

    return EXIT_SUCCESS;
}
