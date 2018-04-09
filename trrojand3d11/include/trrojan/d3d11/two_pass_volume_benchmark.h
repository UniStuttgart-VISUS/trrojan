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

        //static const UINT shader_resource_id = 10000;

        ATL::CComPtr<ID3D11Texture2D> back_faces;
        ATL::CComPtr<ID3D11Texture2D> front_faces;
        ATL::CComPtr<ID3D11RasterizerState> rasteriser_state;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
