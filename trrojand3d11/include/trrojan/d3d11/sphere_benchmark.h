/// <copyright file="sphere_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <unordered_map>

#include "trrojan/camera.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/benchmark_base.h"
#include "trrojan/d3d11/mmpld_base.h"
#include "trrojan/d3d11/random_sphere_base.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a particle rendering benchmark using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API sphere_benchmark : public benchmark_base,
            public mmpld_base, public random_sphere_base {

    public:

        typedef std::uint32_t frame_type;

        static const char *factor_data_set;
        static const char *factor_frame;
        static const char *factor_iterations;
        static const char *factor_method;

        static const char *method_geosprite;
        static const char *method_tesssprite;
        static const char *method_tesssphere;
        static const char *method_tesshemisphere;

        sphere_benchmark(void);

        virtual ~sphere_benchmark(void);

        virtual void optimise_order(configuration_set& inOutConfs);

        /// <inheritdoc />
        virtual std::vector<std::string> required_factors(void) const;

    protected:

        /// <inheritdoc />
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            const std::vector<std::string>& changed);

    private:

        /// <summary>
        /// Declare a GPU buffer without latency, because we need to know the
        /// results immediately and are willing to wait for it.
        /// </summary>
        typedef trrojan::d3d11::gpu_timer<1> gpu_timer_type;

        trrojan::perspective_camera cam;
        ATL::CComPtr<ID3D11Buffer> constant_buffer;
        ATL::CComPtr<ID3D11GeometryShader> geometry_shader;
        ATL::CComPtr<ID3D11InputLayout> input_layout;
        ATL::CComPtr<ID3D11PixelShader> pixel_shader;
        ATL::CComPtr<ID3D11Buffer> vertex_buffer;
        ATL::CComPtr<ID3D11VertexShader> vertex_shader;

    };

}
}
