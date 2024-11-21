#pragma once

#include <iostream>
#include <fstream>

#include <CL/cl.hpp>

class gpu_manager {
private:
    void init_platform();
    void init_device();

public:
    cl::Platform platform;
    cl::Device device;
    cl::Context context;
    cl::CommandQueue queue;

    gpu_manager();
};
