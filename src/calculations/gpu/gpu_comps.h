#pragma once

#include <vector>

#include <CL/cl.hpp>

#include "calculations/computations.h"

/* This, and the arg parser, are the only files where I found OOP to be useful */

/**
 * GPU computation class
 * Defines the computation of absolute difference and sums on the GPU (OpenCL)
 * Uses the static polymorphism technique (CRTP (Curiously Recurring Template Pattern)) to define the interface
 */
class gpu_comps : public computations<gpu_comps> {
    /**
     * TODO: what does this do?
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     */
    template<typename exec_policy>
    static void sort(exec_policy policy, std::vector<decimal> &arr) {
        /* TODO: implement GPU sort */
    }

    /**
     * TODO: what does this do?
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    template<typename exec_policy>
    static void compute_abs_diff(exec_policy policy, const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) {
        /* TODO: implement GPU absolute difference */
    }

    /**
     * TODO: what does this do?
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    template<typename exec_policy>
    static void compute_sums(exec_policy policy, const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
        /* TODO: implement GPU sum and sum of squares */
    }
};






















template <typename potomek>
class predek {
public:
    void compute_vse(int arg1, int arg2) {
        static_cast<potomek *>(this)->implementace(arg1, arg2);
    }
};

class cpu : predek<cpu> {
public:
    void implementace(int arg1, int arg2) {
        ...; /* pouzije pouze arg1 */
    }
};

class gpu : predek<gpu> {
public:
    void implementace(int arg1, int arg2) {
        ...; /* pouzije pouze arg2 */
    }
};

/* Z venku */

int main() {
    std::variant<cpu, gpu> type_of_computation;
    if (gpu) {
        type_of_computation = gpu();
    }
    else {
        type_of_computation = cpu();
    }

    std::visit(
            ... -> type_of_computation.compute_vse(arg1, arg2);
            );
}













