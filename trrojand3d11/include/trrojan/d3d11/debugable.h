/// <copyright file="debugable.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <functional>

#include <Windows.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Defines the interface for classes which have debug rendering
    /// capabilities.
    /// </summary>
    class TRROJAND3D11_API debugable {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debugable(void) = default;

        /// <summary>
        /// Gets the shared staging texture.
        /// </summary>
        virtual HANDLE get_debug_staging_texture(void) = 0;

    protected:

        /// <summary>
        /// Disallow direct instances of the class.
        /// </summary>
        debugable(void) = default;

    };
}
}
