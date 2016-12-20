/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/opencl/export.h"


namespace trrojan {
namespace opencl {

    /// <summary>
    /// Representation of an OpenCL device.
    /// </summary>
    class TRROJANCL_API device : public trrojan::device_base {

    public:

        inline device(void) { }

        virtual ~device(void);

    };

}
}
