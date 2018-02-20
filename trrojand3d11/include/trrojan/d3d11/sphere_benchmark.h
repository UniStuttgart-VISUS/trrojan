/// <copyright file="sphere_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <unordered_map>

#include "trrojan/camera.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/benchmark_base.h"
#include "trrojan/d3d11/rendering_technique.h"
#include "trrojan/d3d11/sphere_data_set.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a particle rendering benchmark using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API sphere_benchmark : public benchmark_base {

    public:

        /// <summary>
        /// Type identifying a frame index.
        /// <s/ummary>
        typedef std::uint32_t frame_type;

        static const char *factor_conservative_depth;
        static const char *factor_data_set;
        static const char *factor_edge_tess_factor;
        static const char *factor_force_float_colour;
        static const char *factor_frame;
        static const char *factor_inside_tess_factor;
        static const char *factor_iterations;
        static const char *factor_method;
        static const char *factor_poly_maximum;
        static const char *factor_poly_minimum;
        static const char *factor_poly_scale;
        static const char *factor_vs_raygen;
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

        typedef random_sphere_base::random_sphere_type random_sphere_type;

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
        /// Gets the identifier for the given rendering method.
        /// </summary>
        /// <param name=""></param>
        /// <returns>The ID of the requested shader, or 0 if no such shader was
        /// found.</returns>
        static shader_id_type get_shader_id(const std::string& method);

        /// <summary>
        /// Gets the identifier for the rendering method specified in the given
        /// configuration.
        /// </summary>
        /// <remarks>
        /// The method only retrieves the method code and method-dependent
        /// features, but not any data-dependent features. These must be added
        /// later.
        /// </remarks>
        /// <param name=""></param>
        /// <returns></returns>
        static shader_id_type get_shader_id(const configuration& config);

        /// <summary>
        /// Gets or creates a rendering technique using the given rendering
        /// method and input data properties set.
        /// </summary>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <returns>A reference to the cached technique.</returns>
        rendering_technique& get_technique(ID3D11Device *device,
            const shader_id_type method, const shader_id_type data,
            const bool isRandomSpheres);

        /// <summary>
        /// Loads the MMPLD frame with the given number from the already opened
        /// MMPLD file.
        /// </summary>
        void load_mmpld_frame(ID3D11Device *dev,
            const shader_id_type shaderCode, const configuration& config);

        /// <summary>
        /// Try processing the data set in <paramref name="config" /> as
        /// configuration for random sphere generation and load
        /// <see cref="data_buffer" /> with the random spheres in case
        /// of success.
        /// </summary>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <param name=""></param>
        void make_random_spheres(ID3D11Device *dev,
            const shader_id_type shaderCode, const configuration& config);

        /// <summary>
        /// Setsthe data poroperties for random particles.
        /// </summary>
        void set_data_properties(const random_sphere_type type,
            const shader_id_type shaderCode);

        /// <summary>
        /// Sets, if necessary (if <c>SPHERE_TECHNIQUE_USE_SRV</c> is set in
        /// <see cref="data_properties" />), the floating point colour flag in
        /// <see cref="data_properties" />.
        /// </summary>
        void set_float_colour_flag(const mmpld_reader::colour_type colour);

        /// <summary>
        /// The camera for computing the transformation matrices.
        /// </summary>
        trrojan::perspective_camera cam;

        /// <summary>
        /// The shader resource view of the transfer function.
        /// </summary>
        rendering_technique::srv_type colour_map;

        /// <summary>
        /// The data set to be rendered.
        /// </summary>
        sphere_data_set data;

        /// <summary>
        /// Buffer holding the data set.
        /// </summary>
        rendering_technique::buffer_type data_buffer;

        /// <summary>
        /// Shader flags for the data in <see cref="data_buffer" />
        /// </summary>
        shader_id_type data_properties;

        /// <summary>
        /// The number of elements in <see cref="data_buffer" />
        /// </summary>
        std::uint32_t data_size;

        /// <summary>
        /// The stride of the elements in <see cref="data_buffer" />
        /// </summary>
        std::uint32_t data_stride;

        /// <summary>
        /// An event query for detecting the end of rendering on the GPU.
        /// </summary>
        ATL::CComPtr<ID3D11Query> done_query;

        /// <summary>
        /// A linear texture sampler state for the transfer function.
        /// </summary>
        rendering_technique::sampler_state_type linear_sampler;

        /// <summary>
        /// The lookup table for the resource IDs of the shaders.
        /// </summary>
        shader_source_map_type shader_resources;

        /// <summary>
        /// Constant buffer for sphere parameters.
        /// </summary>
        rendering_technique::buffer_type sphere_constants;

        /// <summary>
        /// A query for pipeline statistics.
        /// </summary>
        ATL::CComPtr<ID3D11Query> stats_query;

        /// <summary>
        /// Caches rendering techniques already created.
        /// </summary>
        technique_map_type technique_cache;

        /// <summary>
        /// Constant buffer holding parameters for GPU tessellation.
        /// </summary>
        rendering_technique::buffer_type tessellation_constants;

        /// <summary>
        /// Constant buffer holding the current view parameters.
        /// </summary>
        rendering_technique::buffer_type view_constants;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
