#include "merge_sort.h"

void merge(std::vector<double> &arr, size_t left, size_t mid, size_t right) {
    size_t n1 = mid - left + 1;
    size_t n2 = right - mid;

    /* Temporary arrays to hold the left and right halves */
    std::vector<double> leftArr(n1);
    for (size_t i = 0; i < n1; i++)
        leftArr[i] = arr[left + i];
    std::vector<double> rightArr(n2);
    for (size_t i = 0; i < n2; i++)
        rightArr[i] = arr[mid + 1 + i];

    /* Actual merge */
    size_t i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i] <= rightArr[j]) {
            arr[k] = leftArr[i];
            i++;
        } else {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    /* Copy any remaining elements from either of the halves */
    while (i < n1) {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    while (j < n2) {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
}

void my_merge_sort(std::vector<double> &arr) {
    for (size_t size = 1; size < arr.size(); size *= 2) {
        #pragma omp parallel for default(none) shared(arr, size)
        for (size_t left = 0; left < arr.size(); left += 2 * size) {
            size_t mid = std::min(left + size - 1, arr.size() - 1);
            size_t right = std::min(left + 2 * size - 1, arr.size() - 1);

            /* Merge */
            merge(arr, left, mid, right);
        }
    }
}
