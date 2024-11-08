#pragma once

#include <iostream>
#include <vector>
#include <string>

/** Max byte value */
constexpr size_t max_byte_value = 256;
/** 64 KB (buffer size) */
constexpr size_t KB64 = 64 << 10;
/** 1 MB (buffer size) */
constexpr size_t MB1 = 1 << 20;

/**
 * Data structure to store the loaded data
 * Contains three vectors for X, Y and Z data
 */
struct patient_data {
    std::vector<double> x;
    std::vector<double> y;
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
 * Uses a large buffer and minimal memory allocations
 * @param filepath Path to the file
 * @param data Data structure to store the loaded data
 */
void load_data_super_fast(const std::string &filepath, patient_data &data);
