#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <omp.h>

/** Max byte value */
constexpr size_t max_byte_value = 256;
/** 1 KB */
constexpr size_t KB = 1 << 10;
/** 1 MB */
constexpr size_t MB = 1 << 20;

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
 * Load data from a file
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data(const std::string &filepath, patient_data &data);

/**
 * Loads data from a file super fast
 * Uses a large buffer and resizing of vectors and direct indexing
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_super_fast(const std::string &filepath, patient_data &data);

/**
 * Loads data from a file in parallel
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_parallel(const std::string &filepath, patient_data &data);
