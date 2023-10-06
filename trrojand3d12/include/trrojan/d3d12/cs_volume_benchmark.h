// <copyright file="cs_volume_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

//#include "trrojan/d3d12/rendering_technique.h"
#include "trrojan/d3d12/volume_benchmark_base.h"

#include "trrojan/d3d12/gpu_timer.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A D3D12 volume renderer using a compute shader for traversing the rays.
    /// </summary>
    class TRROJAND3D12_API cs_volume_benchmark : public volume_benchmark_base {

    public:

        typedef volume_benchmark_base::frame_type frame_type;
        typedef trrojan::d3d12::gpu_timer gpu_timer_type;
        typedef volume_benchmark_base::info_type info_type;
        typedef volume_benchmark_base::manoeuvre_step_type manoeuvre_step_type;
        typedef volume_benchmark_base::manoeuvre_type manoeuvre_type;
        typedef volume_benchmark_base::point_type point_type;
        typedef volume_benchmark_base::reader_type reader_type;
        typedef volume_benchmark_base::step_size_type step_size_type;
        typedef volume_benchmark_base::viewport_type viewport_type;

        cs_volume_benchmark(void);

        virtual ~cs_volume_benchmark(void) = default;

    protected:

        /// <inheritdoc />
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed);

    private:

        //rendering_technique::uav_type compute_target;
        //ATL::CComPtr<ID3D11Query> done_query;
        //rendering_technique::buffer_type raycasting_constants;
        //rendering_technique technique;
        //rendering_technique::buffer_type view_constants;

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
