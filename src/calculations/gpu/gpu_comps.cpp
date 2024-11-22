#include "calculations/gpu/gpu_comps.h"

gpu_comps::gpu_comps() {
    /* Initialize OpenCL overhead */
    this->platform = init_platform();
    this->device = init_device(platform);
    this->context = cl::Context(device);
    this->queue = cl::CommandQueue(context, device);
    this->program = load_program(context, device, kernel_source);
}
