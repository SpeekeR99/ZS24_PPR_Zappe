#pragma once

#include <vector>

#include <CL/cl.hpp>

#include "calculations/computations.h"
#include "calculations/gpu/gpu.h"
#include "utils/utils.h"

/* This, and the arg parser, are the only files where I found OOP to be useful */

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
     * TODO: what does this do?
     * This is an actual implementation of the "abstract" function in the base class
     * This function has to be implemented in here (.h), because of the template
     * @tparam exec_policy Execution policy (std::execution::seq or std::execution::par)
     * @param policy Execution policy (unused for GPU computations, but needed for the interface)
     * @param arr Array
     */
    template<typename exec_policy>
    void sort(exec_policy policy, std::vector<decimal> &arr) {
        /* TODO: implement GPU sort */
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
    void compute_sums(exec_policy policy, const std::vector<decimal> &arr, decimal &sum, decimal &sum_sq) {
        /* TODO: implement GPU sum and sum of squares */
    }
};
