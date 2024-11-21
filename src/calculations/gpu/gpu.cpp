#include "gpu.h"

cl::Platform init_platform() {
    /* Get number of platforms */
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty())
        throw std::runtime_error("No OpenCL platforms found");

    /* Try to find NVIDIA platform, otherwise use the first one */
    for (const auto& plat : platforms) {
        std::string name = plat.getInfo<CL_PLATFORM_NAME>();
        if (name.find("NVIDIA") != std::string::npos)
            return plat;
    }

    /* If NVIDIA platform was not found, use the first one */
    return platforms.front();
}

cl::Device init_device(cl::Platform &platform) {
    /* Get number of devices */
    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

    if (devices.empty())
        throw std::runtime_error("No OpenCL devices found");

    /* Use the first device */
    return devices.front();
}

cl::Program load_program(cl::Context &context, cl::Device &device, const std::string &kernel_source) {
    /* Create program from source */
    cl::Program program(context, kernel_source);

    /* Try to build it */
    try {
        program.build({device});
    } catch (const std::exception &e) {
        std::cerr << "Build Log:\n" << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << std::endl;
        throw std::runtime_error("Failed to build OpenCL program");
    }

    return program;
}
