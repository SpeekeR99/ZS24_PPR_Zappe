#include "calculations/gpu/gpu_comps.h"

gpu_comps::gpu_comps() {
    /* Initialize OpenCL overhead */
    this->platform = init_platform();
    this->device = init_device(platform);
    this->context = cl::Context(device);
    this->queue = cl::CommandQueue(context, device);
    this->program = load_program(context, device, kernel_source);

    std::cout << this->get_gpu_info() << std::endl;
}

std::string gpu_comps::get_gpu_info() {
    std::string info = "GPU Info:\n";
    info += "Platform: " + platform.getInfo<CL_PLATFORM_NAME>() + "\n";
    info += "Device: " + device.getInfo<CL_DEVICE_NAME>() + "\n";
    return info;
}