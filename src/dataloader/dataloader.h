#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>

#include <execution>

#include "utils/utils.h"

/* Disabling C4996 warning, because I know what I am doing with the old C functions like fopen, strtok, etc. */
#pragma warning(disable : 4996)

/** Max byte value */
constexpr size_t max_byte_value = 256;
/** 1 KB */
constexpr size_t KB = 1 << 10;
/** 1 MB */
constexpr size_t MB = KB << 10;

/**
 * Data structure to store the loaded data
 * Contains three vectors for X, Y and Z data
 */
struct patient_data {
    /** X data */
    std::vector<decimal> x;
    /** Y data */
    std::vector<decimal> y;
    /** Z data */
    std::vector<decimal> z;
};

/**
 * Load data from a file using the standard C++ I/O functions (std::ifstream, std::getline)
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data(const std::string &filepath, patient_data &data);

/**
 * Load data from a file using the standard ANSI C I/O functions (fopen, fscanf, fclose)
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_fast(const std::string &filepath, patient_data &data);

/**
 * Loads data from a file super fast using the standard ANSI C I/O functions (fopen, fscanf, fclose)
 * Uses a large buffer and resizing of vectors and direct indexing
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_super_fast(const std::string &filepath, patient_data &data);

/**
 * Loads data from a file in parallel using the standard ANSI C I/O functions (fopen, fscanf, fclose)
 * Also uses ANSI C string functions (strtok, strcpy) and atof, because for some reason strtod is slow in parallel
 * This function has to be implemented in here (.h), because of the template
 * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
 * @param policy Execution policy
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
template <typename exec_policy>
void load_data_parallel(exec_policy policy, const std::string &filepath, patient_data &data) {
    /* Open the file (in binary mode!) */
    FILE *in_fp = fopen(filepath.c_str(), "rb");
    if (!in_fp) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }

    /* Get the file size */
    fseek(in_fp, 0, SEEK_END);
    const size_t file_size = ftell(in_fp);
    fseek(in_fp, 0, SEEK_SET);

    /* Read the file into a buffer into memory (RAM) */
    char *buffer = new char[file_size];
    fread(buffer, 1, file_size, in_fp);
    fclose(in_fp);

    /* Parse per lines */
    std::vector<std::string_view> lines;
    size_t start_index = 0;  /* Start of the line */
    for (size_t i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            lines.emplace_back(buffer + start_index, i - start_index);
            start_index = i + 1;  /* Skip the newline */
        }
    }

    /* Skip the header */
    lines.erase(lines.begin());

    /* Clean the data */
    data.x.clear();
    data.y.clear();
    data.z.clear();

    /* Resize the vectors to the number of lines -- big advantage here over the previous load */
    size_t num_lines = lines.size();
    data.x.resize(num_lines);
    data.y.resize(num_lines);
    data.z.resize(num_lines);

    /* Prepare for parallelism */
    const auto max_num_threads = std::thread::hardware_concurrency();
    const size_t chunk_size = num_lines / max_num_threads;
    std::vector<size_t> chunk_indices(max_num_threads);
    std::iota(chunk_indices.begin(), chunk_indices.end(), 0);

    std::for_each(policy, chunk_indices.begin(), chunk_indices.end(), [&](const auto i) {
        const size_t start = i * chunk_size;
        /* Final thread may have to handle a little more elements */
        const size_t end = (i == max_num_threads - 1) ? num_lines : (i + 1) * chunk_size;

        char line[max_byte_value];
        /* Parse the lines */
        for (size_t j = start; j < end; j++) {
            /* Copy the line to a buffer */
            strncpy(line, lines[j].data(), lines[j].size());
            char *line_ptr = line;

            /* Find the end of the datetime and move to the numeric data */
            while (*line_ptr && *line_ptr != ',')
                line_ptr++;
            line_ptr++; /* Skip comma */

            /* Parse x, y, z values directly */
            #ifndef _USE_FLOAT
            data.x[j] = std::strtod(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            data.y[j] = std::strtod(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            data.z[j] = std::strtod(line_ptr, &line_ptr);
            #else
            data.x[j] = std::strtof(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            data.y[j] = std::strtof(line_ptr, &line_ptr);
            line_ptr++; /* Skip comma */

            data.z[j] = std::strtof(line_ptr, &line_ptr);
            #endif
        }
    });

    /* Clean up */
    delete[] buffer;
}
