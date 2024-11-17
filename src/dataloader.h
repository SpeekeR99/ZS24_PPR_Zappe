#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <cstring>

#include <omp.h>

#include "utils.h"

/**
 * Data structure to store the loaded data
 * Contains three vectors for X, Y and Z data
 */
struct patient_data {
    /** X data */
    std::vector<double> x;
    /** Y data */
    std::vector<double> y;
    /** Z data */
    std::vector<double> z;
};

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
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_parallel(const std::string &filepath, patient_data &data);
