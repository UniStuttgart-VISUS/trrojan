/// <copyright file="environment.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/environment.h"

#include "trrojan/opencl/export.h"


namespace trrojan {
namespace opencl {

    /// <summary>
    /// 
    /// </summary>
    class TRROJANCL_API environment : public trrojan::environment_base {

    public:

        typedef environment_base::device_list device_list;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline environment(void) : environment_base("opencl") { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~environment(void);

        virtual void get_devices(device_list& dst);
    };

}
}
