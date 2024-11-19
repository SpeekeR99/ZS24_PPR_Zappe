#include "calculations/merge_sort.h"

void merge(std::vector<decimal> &arr, const size_t left, const size_t mid, const size_t right) {
    const size_t n1 = mid - left + 1;
    const size_t n2 = right - mid;

    /* Temporary arrays to hold the left and right halves */
    std::vector<decimal> leftArr(arr.begin() + static_cast<int>(left), arr.begin() + static_cast<int>(mid) + 1);
    std::vector<decimal> rightArr(arr.begin() + static_cast<int>(mid) + 1, arr.begin() + static_cast<int>(right) + 1);

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
