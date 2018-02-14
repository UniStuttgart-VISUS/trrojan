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
#include "trrojan/d3d11/rendering_technique.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a particle rendering benchmark using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API sphere_benchmark : public benchmark_base,
            public mmpld_base, public random_sphere_base {

    public:

        /// <summary>
        /// Type identifying a frame index.
        /// <s/ummary>
        typedef std::uint32_t frame_type;

        static const char *factor_data_set;
        static const char *factor_frame;
        static const char *factor_iterations;
        static const char *factor_method;
        static const char *factor_vs_xfer_function;

        static const char *method_quad_inst;
        static const char *method_poly_inst;
        static const char *method_quad_tess;
        static const char *method_poly_tess;
        static const char *method_adapt_poly_tess;
        static const char *method_spta;
        static const char *method_geo_quad;
        static const char *method_geo_poly;
        static const char *method_sphere_tess;
        static const char *method_adapt_sphere_tess;
        static const char *method_hemisphere_tess;
        static const char *method_adapt_hemisphere_tess;

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        sphere_benchmark(void);

        /// <summary>
        /// Finalise the instance,
        /// </summary>
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

        typedef std::pair<const BYTE *, UINT> shader_source_type;

        typedef std::unordered_map<shader_properties, shader_source_type>
            shader_source_map_type;

        /// <summary>
        /// A lookup table for different rendering techniques and their
        /// variants.
        /// </summary>
        typedef std::unordered_map<std::string,
            rendering_technique::version_list> technique_list;

        /// <summary>
        /// Pack static shader code into an entry of the
        /// <see cref="shader_source_map_type" />.
        /// </summary>
        template<size_t N>
        inline static shader_source_type pack_shader_source(const BYTE(&s)[N]) {
            return std::make_pair(s, static_cast<UINT>(sizeof(s)));
        }

        trrojan::perspective_camera cam;
        ATL::CComPtr<ID3D11ShaderResourceView> colour_map;
        ATL::CComPtr<ID3D11Buffer> constant_buffer;
        ATL::CComPtr<ID3D11DomainShader> domain_shader;
        shader_source_map_type domain_shaders;
        ATL::CComPtr<ID3D11Query> done_query;
        ATL::CComPtr<ID3D11GeometryShader> geometry_shader;
        ATL::CComPtr<ID3D11HullShader> hull_shader;
        shader_source_map_type hull_shaders;
        ATL::CComPtr<ID3D11InputLayout> input_layout;
        ATL::CComPtr<ID3D11SamplerState> linear_sampler;
        std::string method;
        ATL::CComPtr<ID3D11PixelShader> pixel_shader;
        shader_source_map_type pixel_shaders;
        ATL::CComPtr<ID3D11Query> stats_query;
        technique_list techniques;
        ATL::CComPtr<ID3D11Buffer> vertex_buffer;
        ATL::CComPtr<ID3D11VertexShader> vertex_shader;
        shader_source_map_type vertex_shaders;
    };

}
}
