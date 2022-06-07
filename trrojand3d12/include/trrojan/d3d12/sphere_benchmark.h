// <copyright file="sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/sphere_benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Benchmarking of raycasting static spheres.
    /// </summary>
    class TRROJAND3D12_API sphere_benchmark : public sphere_benchmark_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        sphere_benchmark(void);

        /// <summary>
        /// Finalise the instance,
        /// </summary>
        virtual ~sphere_benchmark(void) = default;

    protected:

        /// <inheritdoc />
        void on_device_switch(ID3D12Device *device) override;

        /// <inheritdoc />
        trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) override;

    private:

        ATL::CComPtr<ID3D12Resource> _data;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
