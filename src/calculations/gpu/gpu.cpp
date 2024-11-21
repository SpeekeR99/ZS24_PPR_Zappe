#include "gpu.h"

gpu_manager::gpu_manager() {
    try {
        init_platform();
        init_device();
        this->context = cl::Context(this->device);
        this->queue = cl::CommandQueue(this->context, this->device, CL_QUEUE_PROFILING_ENABLE);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        throw;
    }
}

void gpu_manager::init_platform() {
    /* Get number of platforms */
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty())
        throw std::runtime_error("No OpenCL platforms found");

    /* Try to find NVIDIA platform, otherwise use the first one */
    for (const auto& plat : platforms) {
        std::string name = plat.getInfo<CL_PLATFORM_NAME>();
        if (name.find("NVIDIA") != std::string::npos) {
            this->platform = plat;
            return;
        }
    }

    /* If NVIDIA platform was not found, use the first one */
    this->platform = platforms.front();
}

void gpu_manager::init_device() {
    /* Get number of devices */
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty())
        throw std::runtime_error("No OpenCL devices found");

    /* Use the first device */
    this->device = devices.front();
}
