#include <iostream>
#include <vector>
#include <algorithm>
#include "merge_sort.h"

void printArray(const std::vector<double> &arr) {
    for (auto num : arr)
        std::cout << num << " ";
    std::cout << std::endl;
}

int main() {
    std::vector<double> arr = {38, 27, 43, 3, 9, 82, 10};

    std::cout << "Original array: ";
    printArray(arr);

    double sum = 0, sum_sq = 0;
    my_merge_sort(arr, sum, sum_sq);

    std::cout << "Sorted array: ";
    printArray(arr);

    std::cout << "Sum of array elements: " << sum << std::endl;
    std::cout << "Sum of squares of array elements: " << sum_sq << std::endl;

    return 0;
}
