#include "bitonic_sort.h"

void bitonic_merge(std::vector<double>& arr, size_t low, size_t count, bool ascending) {
    for (size_t k = count / 2; k > 0; k /= 2) { // Iteratively reduce the size of the sequences
        // Parallelize the merging of elements
        for (size_t i = low; i < low + count - k; i++) {
            if ((arr[i] > arr[i + k]) == ascending) {
                std::swap(arr[i], arr[i + k]);
            }
        }
    }
}

void my_bitonic_sort(std::vector<double>& arr, double& sum, double& sum_sq) {
    size_t original_size = arr.size();

    // Calculate the next power of 2 greater than or equal to the array size
    size_t n = 1;
    while (n < original_size)
        n *= 2;

    // Resize the array to the next power of 2, padding with max double values
    if (arr.size() < n)
        arr.resize(n, std::numeric_limits<double>::max());  // Pad with max double values

    // Iteratively perform the bitonic sort
    for (size_t size = 2; size <= n; size *= 2) {  // size doubles at each step
        // Parallelize the outer loop for merging the bitonic subsequences
        for (size_t low = 0; low < n; low += size) {
            // Merge the bitonic sequence
            bitonic_merge(arr, low, size, true);
        }
    }

    // Restore the array size (remove padding)
    arr.resize(original_size);

    // Calculate the sum and sum of squares of the sorted array
//    for (double i : arr) {
//        sum += i;
//        sum_sq += i * i;
//    }
}
