/// <copyright file="output.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include "trrojan/configuration.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Base class for output handlers.
    /// </summary>
    class TRROJANCORE_API output_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~output_base(void);

        // TODO: define the interface.

    };

    /// <summary>
    /// An output handler.
    /// </summary>
    typedef std::shared_ptr<output_base> output;
}
