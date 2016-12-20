/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Base class for devices to be tested.
    /// </summary>
    class TRROJANCORE_API device_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device_base(void);

        // TODO: define the interface.

    };

    /// <summary>
    /// A device.
    /// </summary>
    typedef std::shared_ptr<device_base> device;
}
