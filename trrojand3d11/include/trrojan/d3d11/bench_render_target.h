/// <copyright file="bench_render_target.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/d3d11/render_target.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements an off-screen render target used for actual benchmarking.
    /// </summary>
    class TRROJAND3D11_API bench_render_target : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="device">The device to create the target on.</param>
        bench_render_target(const trrojan::device& device);

        virtual void resize(const unsigned int width,
            const unsigned int height);

    private:

        typedef render_target_base base;

    };
}
}
