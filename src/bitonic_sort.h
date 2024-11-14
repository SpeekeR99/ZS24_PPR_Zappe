#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <execution>

void bitonic_merge(std::vector<double>& arr, size_t low, size_t count, bool ascending);
void my_bitonic_sort(std::vector<double>& arr, double& sum, double& sum_sq);
