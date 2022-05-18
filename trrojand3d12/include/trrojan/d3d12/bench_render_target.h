// <copyright file="bench_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2012 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/device.h"

#include "trrojan/d3d12/render_target.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Implements an off-screen render target used for actual benchmarking.
    /// </summary>
    class TRROJAND3D12_API bench_render_target : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="device">The device to create the target on.</param>
        bench_render_target(const trrojan::device& device);

        virtual void resize(const unsigned int width,
            const unsigned int height) override;

    private:

        typedef render_target_base base;

    };
}
}
