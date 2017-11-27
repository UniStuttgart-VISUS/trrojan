/// <copyright file="util.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/opencl/export.h"

#include <map>

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
    #include "OpenCL/cl.hpp"
#else
	#define CL_HPP_MINIMUM_OPENCL_VERSION 120
	#define CL_HPP_TARGET_OPENCL_VERSION 120
    #include <CL/cl2.hpp>
#endif

namespace trrojan
{
namespace opencl
{
    /// <summary>
    /// The hardware vendor enum.
    /// </summary>
    enum vendor
    {
        VENDOR_UNKNOWN  = (1 << 0),
        VENDOR_AMD      = (1 << 1),
        VENDOR_INTEL    = (1 << 2),
        VENDOR_NVIDIA   = (1 << 3),
        VENDOR_ANY      = (1 << 4),
    };


    /// <summary>
    /// The hardware type enum.
    /// </summary>
    enum hardware_type
    {
        TYPE_DEFAULT        = (1 << 0),     // CL_DEVICE_TYPE_DEFAULT
        TYPE_CPU            = (1 << 1),     // CL_DEVICE_TYPE_CPU
        TYPE_GPU            = (1 << 2),     // CL_DEVICE_TYPE_GPU
        TYPE_ACCELERATOR    = (1 << 3),     // CL_DEVICE_TYPE_ACCELERATOR
        TYPE_CUSTOM         = (1 << 4),     // CL_DEVICE_TYPE_CUSTOM
        TYPE_ALL            = 0xFFFFFFFF,   // CL_DEVICE_TYPE_ALL
    };


    /// <summary>
    /// cl_device_type_amd from cl_ext.h from the AMD APP SDK
    /// </summary>
    typedef union
    {
        struct
        {
            cl_uint type;
            cl_uint data[5];
        } raw;

        struct
        {
            cl_uint type;
            cl_char unused[17];
            cl_char bus;
            cl_char device;
            cl_char function;
        } pcie;
    } cl_device_topology_amd;


    /// <summary>
    /// Helper struct for map initialization.
    /// </summary>
    /// <see href="http://stackoverflow.com/questions/207976/how-to-easily-map-c-enums-to-strings" />
    template<typename T> struct map_init_helper
    {
        T& data;
        map_init_helper(T& d) : data(d) {}

        /// <summary>
        /// Returning *this from operator() allows the chaining of operator(),
        /// like operator<< on std::ostream s.
        /// </summary>
        map_init_helper& operator() (typename T::key_type const& key,
                                     typename T::mapped_type const& value)
        {
            data[key] = value;
            return *this;
        }
    };


namespace   // anonymous namespace defining local class
{
    /// <summary>
    /// OpenCL utility functions.
    /// </summary>
    class TRROJANCL_API util
    {

    public:

        /// <summary>
        /// Map of vendor names.
        /// </summary>
        static std::map<vendor, const char*> _vendor_names;


        /// <summary>
        /// Map of hardware type names.
        /// </summary>
        static std::map<hardware_type, const char*> _type_names;


        /// <summary>
        /// Returns a <see cref="trrojan::map_init_helper" />.
        /// </summary>
        template<typename T> map_init_helper<T> map_init(T& item)
        {
            return map_init_helper<T>(item);
        }


        /// <summary>
        /// Search an input string for a valid vendor name and return the according type.
        /// <param name="s">The string to search for a valid vendor name.</param>
        /// <return>The vendor type if a valid one is found in the string,
        /// VENDOR_UNKNOWN otherwise.</param>
        static vendor get_vendor_from_string(std::string s)
        {
            vendor v = VENDOR_UNKNOWN;
            for (auto &a : _vendor_names)
            {
                if (s.find(a.second) != std::string::npos)
                {
                    v = a.first;
                    break;
                }
                else if (s.find("Advanced Micro Devices") != std::string::npos)
                {
                    v = VENDOR_AMD;
                    break;
                }
            }
            return v;
        }


        /// <summary>
        /// Get an error string based on a string <paramref name="input" /> code.
        /// </summary>
        /// <param name="input">he input string.</param>
        static const std::string get_cl_error_str(std::string input)
        {
            return "Unknown error code.";
        }

        /// <summary>
        /// Return an OpenCL error string based on <paramref name="input" /> error ID.
        /// </summary>
        /// <param name="input">The input error code.</param>
        template<typename T>
        static const std::string get_cl_error_str(T input)
        {
            int error_code = (int)input;
            switch(error_code)
            {
            case CL_DEVICE_NOT_FOUND:
                return "CL_DEVICE_NOT_FOUND";
            case CL_DEVICE_NOT_AVAILABLE:
                return "CL_DEVICE_NOT_AVAILABLE";
            case CL_COMPILER_NOT_AVAILABLE:
                return "CL_COMPILER_NOT_AVAILABLE";
            case CL_MEM_OBJECT_ALLOCATION_FAILURE:
                return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            case CL_OUT_OF_RESOURCES:
                return "CL_OUT_OF_RESOURCES";
            case CL_OUT_OF_HOST_MEMORY:
                return "CL_OUT_OF_HOST_MEMORY";
            case CL_PROFILING_INFO_NOT_AVAILABLE:
                return "CL_PROFILING_INFO_NOT_AVAILABLE";
            case CL_MEM_COPY_OVERLAP:
                return "CL_MEM_COPY_OVERLAP";
            case CL_IMAGE_FORMAT_MISMATCH:
                return "CL_IMAGE_FORMAT_MISMATCH";
            case CL_IMAGE_FORMAT_NOT_SUPPORTED:
                return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            case CL_BUILD_PROGRAM_FAILURE:
                return "CL_BUILD_PROGRAM_FAILURE";
            case CL_MAP_FAILURE:
                return "CL_MAP_FAILURE";
            case CL_MISALIGNED_SUB_BUFFER_OFFSET:
                return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
                return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            case CL_INVALID_VALUE:
                return "CL_INVALID_VALUE";
            case CL_INVALID_DEVICE_TYPE:
                return "CL_INVALID_DEVICE_TYPE";
            case CL_INVALID_PLATFORM:
                return "CL_INVALID_PLATFORM";
            case CL_INVALID_DEVICE:
                return "CL_INVALID_DEVICE";
            case CL_INVALID_CONTEXT:
                return "CL_INVALID_CONTEXT";
            case CL_INVALID_QUEUE_PROPERTIES:
                return "CL_INVALID_QUEUE_PROPERTIES";
            case CL_INVALID_COMMAND_QUEUE:
                return "CL_INVALID_COMMAND_QUEUE";
            case CL_INVALID_HOST_PTR:
                return "CL_INVALID_HOST_PTR";
            case CL_INVALID_MEM_OBJECT:
                return "CL_INVALID_MEM_OBJECT";
            case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
                return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            case CL_INVALID_IMAGE_SIZE:
                return "CL_INVALID_IMAGE_SIZE";
            case CL_INVALID_SAMPLER:
                return "CL_INVALID_SAMPLER";
            case CL_INVALID_BINARY:
                return "CL_INVALID_BINARY";
            case CL_INVALID_BUILD_OPTIONS:
                return "CL_INVALID_BUILD_OPTIONS";
            case CL_INVALID_PROGRAM:
                return "CL_INVALID_PROGRAM";
            case CL_INVALID_PROGRAM_EXECUTABLE:
                return "CL_INVALID_PROGRAM_EXECUTABLE";
            case CL_INVALID_KERNEL_NAME:
                return "CL_INVALID_KERNEL_NAME";
            case CL_INVALID_KERNEL_DEFINITION:
                return "CL_INVALID_KERNEL_DEFINITION";
            case CL_INVALID_KERNEL:
                return "CL_INVALID_KERNEL";
            case CL_INVALID_ARG_INDEX:
                return "CL_INVALID_ARG_INDEX";
            case CL_INVALID_ARG_VALUE:
                return "CL_INVALID_ARG_VALUE";
            case CL_INVALID_ARG_SIZE:
                return "CL_INVALID_ARG_SIZE";
            case CL_INVALID_KERNEL_ARGS:
                return "CL_INVALID_KERNEL_ARGS";
            case CL_INVALID_WORK_DIMENSION:
                return "CL_INVALID_WORK_DIMENSION";
            case CL_INVALID_WORK_GROUP_SIZE:
                return "CL_INVALID_WORK_GROUP_SIZE";
            case CL_INVALID_WORK_ITEM_SIZE:
                return "CL_INVALID_WORK_ITEM_SIZE";
            case CL_INVALID_GLOBAL_OFFSET:
                return "CL_INVALID_GLOBAL_OFFSET";
            case CL_INVALID_EVENT_WAIT_LIST:
                return "CL_INVALID_EVENT_WAIT_LIST";
            case CL_INVALID_EVENT:
                return "CL_INVALID_EVENT";
            case CL_INVALID_OPERATION:
                return "CL_INVALID_OPERATION";
            case CL_INVALID_GL_OBJECT:
                return "CL_INVALID_GL_OBJECT";
            case CL_INVALID_BUFFER_SIZE:
                return "CL_INVALID_BUFFER_SIZE";
            case CL_INVALID_MIP_LEVEL:
                return "CL_INVALID_MIP_LEVEL";
            case CL_INVALID_GLOBAL_WORK_SIZE:
                return "CL_INVALID_GLOBAL_WORK_SIZE";
            case CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR:
                return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
            case CL_PLATFORM_NOT_FOUND_KHR:
                return "CL_PLATFORM_NOT_FOUND_KHR";
                //case CL_INVALID_PROPERTY_EXT:
                //    return "CL_INVALID_PROPERTY_EXT";
            case CL_DEVICE_PARTITION_FAILED_EXT:
                return "CL_DEVICE_PARTITION_FAILED_EXT";
            case CL_INVALID_PARTITION_COUNT_EXT:
                return "CL_INVALID_PARTITION_COUNT_EXT";
//            case CL_INVALID_DEVICE_QUEUE:
//                return "CL_INVALID_DEVICE_QUEUE";
//            case CL_INVALID_PIPE_SIZE:
//                return "CL_INVALID_PIPE_SIZE";

            default:
                return "Unknown error code.";
            }
        }
    };

    std::map<vendor, const char*> trrojan::opencl::util::_vendor_names = {
        {VENDOR_ANY, "any"},
        {VENDOR_AMD, "AMD"},
        {VENDOR_NVIDIA, "NVIDIA"},
        {VENDOR_INTEL, "Intel"},
        {VENDOR_UNKNOWN, "unknown"}};
    }

    std::map<hardware_type, const char*> trrojan::opencl::util::_type_names = {
        {TYPE_DEFAULT, "default"},
        {TYPE_CPU, "CPU"},
        {TYPE_GPU, "GPU"},
        {TYPE_ACCELERATOR, "ACCELERATOR"},
        {TYPE_CUSTOM, "custom"},
        {TYPE_ALL, "all"}};
}
}
