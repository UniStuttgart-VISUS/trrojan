/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/opencl/export.h"

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
    #include "OpenCL/cl.hpp"
#else
    #include <CL/cl.hpp>
#endif

namespace trrojan {
namespace opencl {

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

    private:

        ///
        /// \brief The OpenCL device.
        ///
        cl::Device _cl_device;
    };

}
}
