/// <copyright file="plugin.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>

#include "trrojan/environment.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// This class defines the interface for and a basic implementation of a
    /// plugin.
    /// </summary>
    /// <remarks>
    /// 
    /// </remarks>
    class TRROJANCORE_API plugin_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~plugin_base(void);

    };

    /// <summary>
    /// A plugin.
    /// </summary>
    typedef std::shared_ptr<plugin_base> plugin;
}
