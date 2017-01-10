/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/environment.h"

#include <assert.h>

/*
 * trrojan::opencl::environment::~environment
 */
trrojan::opencl::environment::~environment(void) { }

/*
 * trrojan::opencl::environment::get_devices
 */
size_t trrojan::opencl::environment::get_devices(device_list& dst)
{
    for (auto cl_dev : _opencl.getInfo<CL_CONTEXT_DEVICES>())
    {
        trrojan::opencl::device d;
        d.set_cl_device(cl_dev);
        std::cout << d.name() << " " << d.unique_id() << std::endl;
        //dst.push_back(std::static_pointer_cast<trrojan::device>(d));
    }

    return dst.size();
}

/*
 *
 */
size_t trrojan::opencl::environment::get_platform_cnt()
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.size() == 0)
    {
        throw cl::Error(1, "No OpenCL platforms were found.");
    }

    return platforms.size();
}

/**
 * @brief trrojan::opencl::environment::on_initialise
 * @param cmdLine
 */
void trrojan::opencl::environment::on_initialise(const std::vector<std::string> &cmdLine,
                                                 const int platform_no)
{
    cl_device_type type = CL_DEVICE_TYPE_ALL;
    opencl::vendor vendor = VENDOR_ANY;

    if (cmdLine.size() > 0)
    {
        for (size_t i = 0; i < cmdLine.size() - 1; ++i)
        {
            if (cmdLine.at(i).compare("--device") == 0)
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
            else if (cmdLine.at(i).compare("--vendor") == 0)
            {
                if (cmdLine.at(i + 1).compare("amd") == 0)
                {
                    vendor = VENDOR_AMD;
                }
                else if (cmdLine.at(i + 1).compare("intel") == 0)
                {
                    vendor = VENDOR_INTEL;
                }
                else if (cmdLine.at(i + 1).compare("nvidia") == 0)
                {
                    vendor = VENDOR_NVIDIA;
                }
            }
        }
    }

    create_context(type, vendor, platform_no);
}


cl::Context trrojan::opencl::environment::create_context(cl_device_type type,
                                                         opencl::vendor vendor,
                                                         const int platform_no)
{
    cl::Platform platform = get_platform(type, vendor, platform_no);

    // Use the preferred platform and create a context
    cl_context_properties cps[] = {
                                    CL_CONTEXT_PLATFORM,
                                    (cl_context_properties)(platform)(),
                                    0
                                  };

    try
    {
        cl::Context context = cl::Context(type, cps);
        _opencl = context;
        return context;
    }
    catch (cl::Error error)
    {
        throw cl::Error(1, "Failed to create OpenCL context.");
    }

}


cl::Platform trrojan::opencl::environment::get_platform(cl_device_type type,
                                                        opencl::vendor vendor,
                                                        const int platform_no)
{
    // Get available platforms
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.size() == 0)
    {
        throw cl::Error(1, "No OpenCL platforms were found.");
    }

    int platform_id = -1;
    if (vendor != VENDOR_ANY)
    {
        std::string find;
        switch (vendor)
        {
            case VENDOR_NVIDIA:
                find = "NVIDIA";
            break;
            case VENDOR_AMD:
                find = "Advanced Micro Devices";
            break;
            case VENDOR_INTEL:
                find = "Intel";
            break;
                // TODO: other vendors
            default:
                throw cl::Error(1, "Invalid vendor specified.");
            break;
        }
        for (size_t i = 0; i < platforms.size(); ++i)
        {
            if(platforms[i].getInfo<CL_PLATFORM_VENDOR>().find(find) != std::string::npos)
            {
                try
                {
                    std::vector<cl::Device> devices;
                    platforms[i].getDevices(type, &devices);
                    platform_id = i;
                    break;
                }
                catch (cl::Error error)
                {
                   continue;
                }
            }
        }
    }

    if (platform_id == -1)
    {
        if (vendor != VENDOR_ANY)
        {
            std::cout << "No platform of the specified vendor found. Trying other available platforms."
                      << std::endl;
        }
        assert(platform_no < platforms.size());
        for (size_t i = platform_no; i < platforms.size(); ++i)
        {
            try
            {
                std::vector<cl::Device> devices;
                platforms[i].getDevices(type, &devices);
                platform_id = i;
                break;
            }
            catch (cl::Error error)
            {
               continue;
            }
        }
    }

    if (platform_id == -1)
    {
        throw cl::Error(1, "No compatible OpenCL platform found.");
    }

    cl::Platform platform = platforms[platform_id];
    std::cout << "Using platform vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>() << std::endl;
    return platform;
}


void trrojan::opencl::environment::on_finalise() noexcept
{
}
