/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/opencl/export.h"

//#include "trrojan/opencl/environment.h"

#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
    #include "OpenCL/cl.hpp"
#else
    #include <CL/cl.hpp>
#endif

namespace trrojan {
namespace opencl {

    /// <summary>
    /// The hardware vendor enum.
    /// </summary>
    enum vendor
    {
        VENDOR_UNKNOWN,
        VENDOR_NVIDIA,
        VENDOR_AMD,
        VENDOR_INTEL,
        VENDOR_ANY,
    };

    /// <summary>
    /// The hardware type enum.
    /// </summary>
    enum hardware_type
    {
        TYPE_OTHER,
        TYPE_GPU,
        TYPE_CPU,
        TYPE_ACCELERATOR,
        TYPE_ANY,
    };

    /// <summary>
    /// Representation of an OpenCL device.
    /// </summary>
    class TRROJANCL_API device : public trrojan::device_base {

    public:

        typedef std::shared_ptr<device> pointer;

        inline device(void) { }

        device(cl::Device d);

        virtual ~device(void);

        void set_cl_device(cl::Device d);

        inline const cl::Device &get() const
        {
            return _cl_device;
        }

        trrojan::opencl::vendor get_vendor() const;

        trrojan::opencl::hardware_type get_type() const;

    private:

        ///
        /// \brief The OpenCL device.
        ///
        cl::Device _cl_device;
        trrojan::opencl::vendor _vendor;
        trrojan::opencl::hardware_type _type;
    };

}
}
