#include "merge_sort.h"

void merge(std::vector<double> &arr, size_t left, size_t mid, size_t right) {
    size_t n1 = mid - left + 1;
    size_t n2 = right - mid;

    /* Temporary arrays to hold the left and right halves */
    std::vector<double> leftArr(arr.begin() + static_cast<int>(left), arr.begin() + static_cast<int>(mid) + 1);
    std::vector<double> rightArr(arr.begin() + static_cast<int>(mid) + 1, arr.begin() + static_cast<int>(right) + 1);

    /* Actual merge */
    size_t i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
        arr[k++] = leftArr[i] <= rightArr[j] ? leftArr[i++] : rightArr[j++];

    /* Copy any remaining elements from either of the halves */
    while (i < n1)
        arr[k++] = leftArr[i++];
    while (j < n2)
        arr[k++] = rightArr[j++];
}

void merge_sort(std::vector<double> &arr) {
    /* For each subarray size, basically a stride (bottom up approach) */
    for (size_t size = 1; size < arr.size(); size *= 2) {
        /* For each pair of sub-arrays -- left and right -- sort and merge them */
        #pragma omp parallel for default(none) shared(arr, size)
        for (size_t left = 0; left < arr.size(); left += 2 * size) {
            size_t mid = std::min(left + size - 1, arr.size() - 1);
            size_t right = std::min(left + 2 * size - 1, arr.size() - 1);

            /* Sort and merge the pair */
            merge(arr, left, mid, right);
        }
    }
}
