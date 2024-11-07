#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

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
void load_data(std::string filepath, patient_data &data);
