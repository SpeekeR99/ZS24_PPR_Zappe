#pragma once

#include <algorithm>
#include <vector>

#include <CL/cl.hpp>

#include "calculations/computations.h"
#include "calculations/gpu/gpu.h"
#include "utils/utils.h"

/* This, and the arg parser, are the only files where I found OOP to be useful */

/** Local size for the sum reduce kernel */
constexpr size_t local_size = 256;

/**
 * GPU computation class
 * Defines the computation of absolute difference and sums on the GPU (OpenCL)
 * Uses the static polymorphism technique (CRTP (Curiously Recurring Template Pattern)) to define the interface
 */
class gpu_comps : public computations<gpu_comps> {
private:
    /** OpenCL Platform */
    cl::Platform platform;
    /** OpenCL Device */
    cl::Device device;
    /** OpenCL Context */
    cl::Context context;
    /** OpenCL Command Queue */
    cl::CommandQueue queue;
    /** OpenCL Program */
    cl::Program program;

public:
    /**
     * Constructor
     * Initializes OpenCL overhead
     */
    gpu_comps();

    /**
     * Get GPU information
     * @return String with GPU information
     */
    std::string get_gpu_info();

    /**
     * Uses merge kernel to sort the array on the GPU
     * This is not an optimal GPU sorting algorithm, but it works
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     */
    template<typename exec_policy>
    void sort(exec_policy policy, std::vector<decimal> &arr) {
        (void) policy;  /* Supress warning about unused policy */

        const auto n = arr.size();

        /* Create buffers */
        cl::Buffer buffer_arr(this->context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(decimal) * n, arr.data());
        cl::Buffer buffer_temp(this->context, CL_MEM_READ_WRITE, sizeof(decimal) * n);

        /* Prepare kernel */
        cl::Kernel kernel_merge_sort(this->program, "merge");

        /* Iterate over the array with increasing sub-array sizes (stride) */
        for (size_t size = 1; size < arr.size(); size *= 2) {
            /* Set kernel arguments */
            kernel_merge_sort.setArg(0, buffer_arr);
            kernel_merge_sort.setArg(1, buffer_temp);
            kernel_merge_sort.setArg(2, static_cast<int>(size));
            kernel_merge_sort.setArg(3, static_cast<int>(n));

            /* Calculate global size */
            size_t global_size = (n + 2 * size - 1) / (2 * size);

            /* Execute kernel */
            this->queue.enqueueNDRangeKernel(kernel_merge_sort, cl::NullRange, cl::NDRange(global_size), cl::NullRange);
            this->queue.finish();
        }

        /* Read result */
        this->queue.enqueueReadBuffer(buffer_arr, CL_TRUE, 0, sizeof(decimal) * n, arr.data());

        std::cout << ((std::is_sorted(arr.begin(), arr.end())) ? "Sorted" : "NOT SORTED!") << std::endl;
    }

    /**
     * Compute absolute difference between each element and the median on the GPU
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     * @param median Median of the array
     * @return Absolute difference between each element and the median
     */
    template<typename exec_policy>
    void compute_abs_diff(exec_policy policy, const std::vector<decimal> &arr, decimal median, std::vector<decimal> &diff) {
        (void) policy;  /* Supress warning about unused policy */

        const auto n = arr.size();

        /* Create buffers */
        cl::Buffer buffer_arr(this->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(decimal) * n, const_cast<decimal *>(arr.data()));
        cl::Buffer buffer_diff(this->context, CL_MEM_WRITE_ONLY, sizeof(decimal) * n);

        /* Prepare kernel and arguments */
        cl::Kernel kernel(this->program, "abs_diff");
        kernel.setArg(0, buffer_arr);
        kernel.setArg(1, buffer_diff);
        kernel.setArg(2, median);

        /* Execute kernel */
        this->queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(n));
        this->queue.finish();

        /* Read result */
        this->queue.enqueueReadBuffer(buffer_diff, CL_TRUE, 0, sizeof(decimal) * n, diff.data());
    }

    /**
     * Compute sum of elements in the array and sum of squared elements in the array on the GPU
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     * @param sum Sum of the array elements
     * @param sum_sq Sum of squares of the array elements
     */
    template<typename exec_policy>
    void compute_sums(exec_policy policy, const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
        (void) policy;  /* Supress warning about unused policy */

        const auto n = arr.size();

        /* Partial results buffer -- equivalent to local_sums in my CPU implementation */
        std::vector<decimal> sums((n + local_size - 1) / local_size);
        std::vector<decimal> sums_sq((n + local_size - 1) / local_size);

        /* Create buffers */
        cl::Buffer buffer_arr(this->context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(decimal) * n, const_cast<decimal *>(arr.data()));
        cl::Buffer buffer_sums(this->context, CL_MEM_WRITE_ONLY, sizeof(decimal) * sums.size());
        cl::Buffer buffer_sums_sq(this->context, CL_MEM_WRITE_ONLY, sizeof(decimal) * sums_sq.size());

        /* Prepare kernel and arguments */
        cl::Kernel kernel(this->program, "reduce_sum");
        kernel.setArg(0, buffer_arr);
        kernel.setArg(1, buffer_sums);
        kernel.setArg(2, buffer_sums_sq);
        kernel.setArg(3, static_cast<int>(n));

        /* Execute kernel */
        size_t global_size = ((n + local_size - 1) / local_size) * local_size;
        this->queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(global_size), cl::NDRange(local_size));
        this->queue.finish();

        /* Read the partial results */
        this->queue.enqueueReadBuffer(buffer_sums, CL_TRUE, 0, sizeof(decimal) * sums.size(), sums.data());
        this->queue.enqueueReadBuffer(buffer_sums_sq, CL_TRUE, 0, sizeof(decimal) * sums_sq.size(), sums_sq.data());

        /* Sum the partial results */
        for (size_t i = 0; i < sums.size(); i++) {
            sum += sums[i];
            sum_sq += sums_sq[i];
        }
    }
};
