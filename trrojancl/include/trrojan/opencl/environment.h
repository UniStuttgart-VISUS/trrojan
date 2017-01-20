/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/opencl/export.h"

#include "trrojan/opencl/device.h"

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX) // untested
    #define GL_SHARING_EXTENSION "cl_APPLE_gl_sharing"
    #include "OpenCL/cl.hpp"
#else
    #define GL_SHARING_EXTENSION "cl_khr_gl_sharing"
    #include <CL/cl.hpp>

    #if _WIN32
        #include <gl/gl.h>
    #else
        #include <GL/glx.h>
    #endif
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <set>

namespace trrojan
{
namespace opencl
{
    /// <summary>
    /// The hardware vendor enum.
    /// </summary>
    enum vendor
    {
        VENDOR_ANY,
        VENDOR_NVIDIA,
        VENDOR_AMD,
        VENDOR_INTEL
    };

    /// <summary>
    /// Garbage collector class for OpenCL memory objects.
    /// </summary>
    class garbage_collector
    {
    public:
        void add_mem_object(cl::Memory *mem)
        {
            mem_objects.insert(mem);
        }

        void del_mem_object(cl::Memory *mem)
        {
            mem_objects.erase(mem);
            delete mem;
            mem = NULL;
        }

        void del_all()
        {
            std::set<cl::Memory *>::iterator it;
            for(it = mem_objects.begin(); it != mem_objects.end(); ++it)
            {
                cl::Memory* mem = *it;
                delete (mem);
                mem = NULL;
            }
            mem_objects.clear();
        }

        virtual ~garbage_collector()
        {
            del_all();
        }

    private:
        std::set<cl::Memory*> mem_objects;
    };

    struct properties
    {
        cl::Platform platform;
        std::vector<cl::Device> devices;
        cl::Context context;
        cl::CommandQueue queue;
        cl::Program program;
    };

    /// <summary>
    /// OpenCL environment and basic utility class.
    /// </summary>
    /// <remarks>
    /// This implementation is related and uses some code of Eric Smistad's OpenCL utilities:
    /// https://www.eriksmistad.no/opencl-utilities/
    /// </remarks>
    class TRROJANCL_API environment : public trrojan::environment_base
    {

    public:

        typedef environment_base::device_list device_list;

        typedef std::shared_ptr<environment> pointer;

        /// <summary>
        /// Returns the number of available OpenCL platforms.
        /// </summary>
        static size_t get_platform_names(std::vector<std::string> &names);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(std::string name) : environment_base(name) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        ///
        /// \brief Get a list of available devices
        /// \param dst device list
        /// \return size
        ///
        virtual size_t get_devices(device_list& dst);

        /// TODO
        /// \brief get_context
        /// \return
        ///
        const properties &get_properties() const;

        /// <summary>
        /// Get the garbage collector for OpenCL memory objects.
        /// </summary>
        garbage_collector &get_garbage_collector();

        /// <summary>
        /// Generate an OpenCL program from <paramref name="source" />.
        /// </summary>
        /// <param name="source">The OpenCL C program source code.</param>
        void generate_program(const cl::Program::Sources source);

        ///
        /// \brief generate_queue
        /// \param dev
        /// \param prop
        ///
        void create_queue(const cl::Device &dev, const cl_command_queue_properties prop);

        ///
        /// \brief on_initialise
        /// \param cmdLine
        ///
        virtual void on_initialise(const std::vector<std::string> &cmdLine, const int platform_no = 0);
        virtual void on_initialize(const std::vector<std::string> &cmdLine, const int platform_no = 0)
        {
            on_initialise(cmdLine, platform_no);
        }

        ///
        /// \brief on_finalise
        ///
        virtual void on_finalise() noexcept;

    private:

        ///
        /// \brief Create an OpenCL context with a specific device typy of a vendor.
        /// \param type The device type (CPU, GPU, ACCELERATOR)
        /// \param vendor The device vendor.
        /// \return The selected OpenCL context.
        /// \throws cl::Error if no context is found with the specified parameters.
        ///
        cl::Context create_context(cl_device_type type, opencl::vendor vendor, const int platform_no);

        /// <summary>
        /// Create an OpenCL context with a specific device typy of a vendor with an OpenGL context.
        /// Note that you need a valid OpenGL window from which you can derive the context.
        /// </summary>
        cl::Context create_CLGL_context(cl_device_type type,
                                        opencl::vendor vendor,
                                        const int platform_no);

        /// <summary>
        /// Returns a valid GLCL interoperation device if one exists.
        /// Throws an OpenCL error otherwise.
        /// </summary>
        cl::Device get_valid_GLCL_device(cl::Platform platform,
                                         cl_context_properties* properties);

        /// <summary>
        /// Get a OpenCL platform of a specific device type and vendor
        /// </summary>
        /// <param name="type">The device type (CPU, GPU, ACCELERATOR)</param>
        /// <param name="vendor">The device vendor.</param>
        /// <return>The selected OpenCL platform.</return>
        /// <throws>cl::Error if no platform is found with the specified parameters.</throws>
        cl::Platform get_platform(cl_device_type type, opencl::vendor vendor, const int platform_no);

        ///
        /// OpenCL properties
        ///
        properties _prop;

        /// <summary>
        /// Manual garbage collector for OpenCL memory objects.
        /// </summary>
        mutable garbage_collector _gc;
    };
}
}
