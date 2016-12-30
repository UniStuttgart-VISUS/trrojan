/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/environment.h"


/*
 * trrojan::opencl::environment::~environment
 */
trrojan::opencl::environment::~environment(void) { }


/*
 * trrojan::opencl::environment::get_devices
 */
size_t trrojan::opencl::environment::get_devices(device_list& dst)
{
    return 0;
}

/*
 *
 */
size_t trrojan::opencl::environment::get_platform_cnt()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if(platforms.size() == 0)
    {
        throw cl::Error(1, "No OpenCL platforms were found.");
    }

    return platforms.size();
}

void trrojan::opencl::environment::on_initialise(const std::vector<std::string> &cmdLine)
{
    cl_device_type type = CL_DEVICE_TYPE_ALL;
    opencl::vendor vendor = VENDOR_ANY;

    for(size_t i = 0; i < cmdLine.size() - 1; ++i)
    {
        if(cmdLine.at(i).compare("--device") == 0)
        {
            if(cmdLine.at(i + 1).compare("cpu") == 0)
            {
                type = CL_DEVICE_TYPE_CPU;
            }
            else if(cmdLine.at(i + 1).compare("gpu") == 0)
            {
                type = CL_DEVICE_TYPE_GPU;
            }
            else if(cmdLine.at(i + 1).compare("accelerator") == 0)
            {
                type = CL_DEVICE_TYPE_ACCELERATOR;
            }
        }
        else if(cmdLine.at(i).compare("--vendor") == 0)
        {
            if(cmdLine.at(i + 1).compare("amd") == 0)
            {
                vendor = VENDOR_AMD;
            }
            else if(cmdLine.at(i + 1).compare("intel") == 0)
            {
                vendor = VENDOR_INTEL;
            }
            else if(cmdLine.at(i + 1).compare("nvidia") == 0)
            {
                vendor = VENDOR_NVIDIA;
            }
        }
    }

    //create_CL_context(type, vendor);
}

//void trrojan::opencl::environment::on_finalise()
//{
//}
