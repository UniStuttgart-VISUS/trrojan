/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/opencl/export.h"

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
    #include "OpenCL/cl.hpp"
#else
    #include <CL/cl.hpp>
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
    class GC
    {
        typedef std::shared_ptr<cl::Memory> cl_mem_ptr;
    public:
        void add_mem_object(cl_mem_ptr mem);
        void del_mem_object(cl_mem_ptr mem);
        void del_all();
        ~GC();
    private:
        std::set<cl_mem_ptr> mem_objects;
    };

    /// <summary>
    ///
    /// </summary>
    typedef struct OpenCL
    {
        cl::Context context;
        cl::CommandQueue queue;
        cl::Program program;
        cl::Device device;
        cl::Platform platform;
        GC gc;
    } OpenCL;

    /// <summary>
    /// 
    /// </summary>
    class TRROJANCL_API environment : public trrojan::environment_base
    {

    public:

        typedef environment_base::device_list device_list;

        /// <summary>
        /// Returns the number of available OpenCL platforms.
        /// </summary>
        static size_t get_platform_cnt();

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(void) : environment_base("opencl") { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        virtual void get_devices(device_list& dst);

        virtual void on_initialise(const std::vector<std::string> &cmdLine);

//        virtual void on_finalise();
    };

}
}
