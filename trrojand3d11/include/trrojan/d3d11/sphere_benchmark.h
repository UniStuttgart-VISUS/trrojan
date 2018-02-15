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
        /// The resource IDs of the shaders for a specific rendering technique.
        /// </summary>
        struct shader_resources {
            std::uint16_t vertex_shader;
            std::uint16_t hull_shader;
            std::uint16_t domain_shader;
            std::uint16_t geometry_shader;
            std::uint16_t pixel_shader;
        };

        /// <summary>
        /// Declare a GPU buffer without latency, because we need to know the
        /// results immediately and are willing to wait for it.
        /// </summary>
        typedef trrojan::d3d11::gpu_timer<1> gpu_timer_type;

        /// <summary>
        /// An identifier for a single shader variant.
        /// </summary>
        typedef std::uint64_t shader_id_type;

        /// <summary>
        /// A hash table for all shader sources.
        /// </summary>
        typedef std::unordered_map<shader_id_type, shader_resources>
            shader_source_map_type;

        /// <summary>
        /// A hash table for rendering techniques.
        /// </summary>
        typedef std::unordered_map<shader_id_type, rendering_technique>
            technique_map_type;

        /// <summary>
        /// Gets the identifier for the given rendering method and input+feature
        /// set.
        /// </summary>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <returns>The ID of the requested shader, or 0 if no such shader was
        /// found.</returns>
        static shader_id_type get_shader_id(const std::string& method,
            const shader_id_type features);

        /// <summary>
        /// Gets or creates a rendering technique using the given rendering
        /// method and input+feature set.
        /// </summary>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <returns>A reference to the cached technique.</returns>
        rendering_technique& get_technique(const std::string& method,
            const shader_id_type features);

        /// <summary>
        /// The camera for computing the transformation matrices.
        /// </summary>
        trrojan::perspective_camera cam;

        /// <summary>
        /// The lookup table for the resource IDs of the shaders.
        /// </summary>
        shader_source_map_type shaderResources;

        /// <summary>
        /// Caches rendering techniques already created.
        /// </summary>
        technique_map_type techniqueCache;

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
        ATL::CComPtr<ID3D11Buffer> vertex_buffer;
        ATL::CComPtr<ID3D11VertexShader> vertex_shader;
        shader_source_map_type vertex_shaders;
    };

}
}
