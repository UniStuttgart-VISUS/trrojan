/// <copyright file="two_pass_volume_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

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
            const std::vector<std::string>& changed);

    private:

        static const UINT compute_shader_resource_id = 20002;
        static ID3D11RenderTargetView *const empty_rtv;
        static ID3D11ShaderResourceView *const  empty_srv;
        static const UINT pixel_shader_resource_id = 20001;
        static const UINT ray_slot = 2;
        static const UINT vertex_shader_resource_id = 20000;

        void begin_ray_pass(ID3D11DeviceContext *ctx,
            const viewport_type& viewport);

        void begin_volume_pass(ID3D11DeviceContext *ctx);

        rendering_technique::srv_type ray_source;
        ATL::CComPtr<ID3D11RenderTargetView> ray_target;
        rendering_technique ray_technique;
        rendering_technique::buffer_type raycasting_constants;
        rendering_technique::buffer_type view_constants;
        rendering_technique volume_technique;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
