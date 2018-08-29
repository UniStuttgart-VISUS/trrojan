/// <copyright file="environment.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/environment.h"
#include "trrojan/log.h"

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
    for (auto cl_dev : _prop.context.getInfo<CL_CONTEXT_DEVICES>())
    {
        opencl::device d;
        d.set_cl_device(cl_dev);
        _prop.devices.push_back(cl_dev);

        dst.push_back(std::make_shared<opencl::device>(d));
    }

    return dst.size();
}


/*
 * trrojan::opencl::environment::get_properties
 */
const trrojan::opencl::properties &trrojan::opencl::environment::get_properties() const
{
    return this->_prop;
}


/*
 * trrojan::opencl::environment::generate_program
 */
void trrojan::opencl::environment::generate_program(const cl::Program::Sources source)
{
    this->_prop.program = cl::Program(this->_prop.context, source, NULL);
}


/**
 * trrojan::opencl::environment::create_queue
 */
void trrojan::opencl::environment::create_queue(const cl::Device &dev,
                                                const cl_command_queue_properties prop)
{
    this->_prop.queue = cl::CommandQueue(this->_prop.context, dev, prop);
}


/*
 * trrojan::opencl::environment::get_platform_names
 */
size_t trrojan::opencl::environment::get_platform_names(std::vector<std::string> &names)
{
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);
    if (platforms.size() == 0)
    {
        throw cl::Error(1, "No OpenCL platforms were found.");
    }
    for (auto& p : platforms)
    {
        names.push_back(p.getInfo<CL_PLATFORM_NAME>());
    }
    return platforms.size();
}


/**
 * trrojan::opencl::environment::on_initialise
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
    _prop.platform = platform;

    // Use the preferred platform and create a context
    cl_context_properties cps[] = {
                                    CL_CONTEXT_PLATFORM,
                                    (cl_context_properties)(platform)(),
                                    0
                                  };

    try
    {
        _prop.context = cl::Context(type, cps);
        std::string vendor_name = platform.getInfo<CL_PLATFORM_VENDOR>();
        _prop.vendor = util::get_vendor_from_string(vendor_name);
        return _prop.context;
    }
    catch (cl::Error error)
    {
        throw cl::Error(1, "Failed to create OpenCL context.");
    }
}


cl::Context trrojan::opencl::environment::create_CLGL_context(cl_device_type type,
                                                              opencl::vendor vendor,
                                                              const int platform_no)
{
    // We need a valid window to create a shared context

    cl::Platform platform = get_platform(type, vendor, platform_no);
    std::vector<cl::Device> interop_device;

#if defined(__APPLE__) || defined(__MACOSX)
    // Apple (untested)
    cl_context_properties cps[] = {
       CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
       (cl_context_properties)CGLGetShareGroup(CGLGetCurrentContext()),
       0
    };
#else
  #ifdef _WIN32
      // Windows
      cl_context_properties cps[] = {
          CL_GL_CONTEXT_KHR,
          (cl_context_properties)wglGetCurrentContext(),
          CL_WGL_HDC_KHR,
          (cl_context_properties)wglGetCurrentDC(),
          CL_CONTEXT_PLATFORM,
          (cl_context_properties)(platform)(),
          0
      };
  #else // UNIX
      cl_context_properties cps[] = {
          CL_GL_CONTEXT_KHR,
          (cl_context_properties)glXGetCurrentContext(),
          CL_GLX_DISPLAY_KHR,
          (cl_context_properties)glXGetCurrentDisplay(),
          CL_CONTEXT_PLATFORM,
          (cl_context_properties)(platform)(),
          0
      };
  #endif
#endif

    try
    {
        platform.getDevices(type, &(_prop.devices));

        // If there is more than one CL device, find out which one is associated with GL context.
        if(_prop.devices.size() > 1u)
        {
#if !(defined(__APPLE__) || defined(__MACOSX))
            interop_device.push_back(get_valid_GLCL_device(platform, cps));
            _prop.context = cl::Context(interop_device, cps);
#else
            _prop.context = cl::Context(type, cps);
#endif
        }
        else
        {
            _prop.context = cl::Context(type, cps);
        }

        return _prop.context;
    }
    catch(cl::Error error)
    {
        throw error;
    }
}


cl::Device trrojan::opencl::environment::get_valid_GLCL_device(cl::Platform platform,
                                                               cl_context_properties* properties)
{
    cl_device_id interop_device_id;

    int status;
    size_t device_size = 0;

    // Load extension function call
    clGetGLContextInfoKHR_fn glGetGLContextInfo_func =
            (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(NULL,
                                                                               "clGetGLContextInfoKHR");

    // Ask for the CL device associated with the GL context
    status = glGetGLContextInfo_func(properties,
                                     CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR,
                                     sizeof(cl_device_id),
                                     &interop_device_id,
                                     &device_size);

    if(device_size == 0)
    {
        throw cl::Error(1, "No GLGL devices found for current platform");
    }

    if(status != CL_SUCCESS)
    {
        throw cl::Error(1, "Could not get CLGL interop device for the current platform. Failure occured during call to clGetGLContextInfoKHR.");
    }

    return cl::Device(interop_device_id);
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
    if (vendor != VENDOR_ANY)   // find specified vendor
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

    if (platform_id == -1)  // if none of the specified vendors were found try to find others
    {
        if (vendor != VENDOR_ANY)
        {
            log::instance().write_line(log_level::warning, "No platform of the specified vendor"
                                       "found. Trying other available platforms.");
        }
        assert(platform_no < static_cast<int>(platforms.size()));
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
        throw cl::Error(1, "No compatible OpenCL platform found.");

    cl::Platform platform = platforms[platform_id];
    return platform;
}


void trrojan::opencl::environment::on_finalise() noexcept
{
}
