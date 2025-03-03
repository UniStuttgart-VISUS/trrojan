﻿// <copyright file="sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
        /// Type for edge tessellation factors.
        /// </summary>
        typedef std::array<float, 4> edge_tess_factor_type;

        /// <summary>
        /// Type identifying a frame index.
        /// <s/ummary>
        typedef std::uint32_t frame_type;

        /// <summary>
        /// Typoe for inside tessellation factors.
        /// </summary>
        typedef std::array<float, 2> inside_tess_factor_type;

        static const char *factor_adapt_tess_maximum;
        static const char *factor_adapt_tess_minimum;
        static const char *factor_adapt_tess_scale;
        static const char *factor_conservative_depth;
        static const char *factor_data_set;
        static const char *factor_edge_tess_factor;
        static const char *factor_fit_bounding_box;
        static const char *factor_force_float_colour;
        static const char *factor_frame;
        static const char *factor_gpu_counter_iterations;
        static const char *factor_hemi_tess_scale;
        static const char *factor_inside_tess_factor;
        static const char *factor_method;
        static const char *factor_min_prewarms;
        static const char *factor_min_wall_time;
        static const char *factor_poly_corners;
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

        /// <inheritdoc />
        virtual trrojan::result on_run(d3d11::device& device,
            const configuration& config,
            power_collector::pointer& powerCollector,
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
        /// The type to specify data set properties
        /// </summary>
        typedef sphere_data_set_base::properties_type data_properties_type;

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
        /// Formats the shader ID as it is formatted in the names of the shader
        /// files.
        /// </summary>
        /// <remarks>
        /// This method can be used to build file names for the CSOs of the
        /// shader if the embedded resources cannot be used.
        /// </remarks>
        /// <param name="id">The ID of the shader as obtained from
        /// <see cref="get_shader_id" />.</param>
        /// <returns>The string representation of the shader ID.</returns>
        static std::string get_shader_file_id(const shader_id_type id);

        /// <summary>
        /// Gets the identifier for the given rendering method.
        /// </summary>
        /// <param name="method"></param>
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
        /// <param name="config"></param>
        /// <returns></returns>
        static shader_id_type get_shader_id(const configuration& config);

        /// <summary>
        /// Answer whether all bits of <paramref name="technique" /> are set
        /// in <see cref="shaderCode" />.
        /// </summary>
        static inline bool is_technique(const shader_id_type shaderCode,
                const shader_id_type technique) {
            return ((shaderCode & technique) == technique);
        }

        /// <summary>
        /// Check whether the currently loaded data are compatible with the
        /// requirements specified in the given shader code.
        /// </summary>
        bool check_data_compatibility(const shader_id_type shaderCode);

        /// <summary>
        /// Retrieves the properties of the currently loaded data set and erases
        /// all flags which are not relevant for the given shader technique.
        /// </summary>
        data_properties_type get_data_properties(
            const shader_id_type shaderCode);

        /// <summary>
        /// Gets or creates a rendering technique using the given rendering
        /// method and input data properties set.
        /// </summary>
        /// <param name=""></param>
        /// <param name=""></param>
        /// <returns>A reference to the cached technique.</returns>
        rendering_technique& get_technique(ID3D11Device *device,
            shader_id_type shaderCode);

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
        /// An event query for detecting the end of rendering on the GPU.
        /// </summary>
        winrt::com_ptr<ID3D11Query> done_query;

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
        winrt::com_ptr<ID3D11Query> stats_query;

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
