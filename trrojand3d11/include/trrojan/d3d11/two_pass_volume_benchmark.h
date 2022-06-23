// <copyright file="two_pass_volume_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d11/rendering_technique.h"
#include "trrojan/d3d11/volume_benchmark_base.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Two-pass volume renderer which uses the graphics pipeline to compute all
    /// rays at once.
    /// </summary>
    class TRROJAND3D11_API two_pass_volume_benchmark
            : public volume_benchmark_base {

    public:

        typedef volume_benchmark_base::frame_type frame_type;
        typedef trrojan::d3d11::gpu_timer<1> gpu_timer_type;
        typedef volume_benchmark_base::info_type info_type;
        typedef volume_benchmark_base::manoeuvre_step_type manoeuvre_step_type;
        typedef volume_benchmark_base::manoeuvre_type manoeuvre_type;
        typedef volume_benchmark_base::point_type point_type;
        typedef volume_benchmark_base::reader_type reader_type;
        typedef volume_benchmark_base::step_size_type step_size_type;
        typedef volume_benchmark_base::viewport_type viewport_type;

        two_pass_volume_benchmark(void);

        virtual ~two_pass_volume_benchmark(void) = default;

    protected:

        /// <inheritdoc />
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            power_collector::pointer& powerCollector,
            const std::vector<std::string>& changed);

    private:

        static const UINT compute_shader_resource_id = 20002;
        static const UINT first_ray_slot = 2;
        static const UINT pixel_shader_resource_id = 20001;
        static const UINT vertex_shader_resource_id = 20000;

        void begin_ray_pass(ID3D11DeviceContext *ctx,
            const viewport_type& viewport);

        void begin_volume_pass(ID3D11DeviceContext *ctx);

        ATL::CComPtr<ID3D11Query> done_query;
        rendering_technique::srv_type entry_source;
        ATL::CComPtr<ID3D11RenderTargetView> entry_target;
        rendering_technique::srv_type ray_source;
        ATL::CComPtr<ID3D11RenderTargetView> ray_target;
        rendering_technique ray_technique;
        rendering_technique::buffer_type raycasting_constants;
        rendering_technique::buffer_type view_constants;
        rendering_technique volume_technique;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
