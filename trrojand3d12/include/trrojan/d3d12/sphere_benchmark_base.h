// <copyright file="sphere_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <unordered_map>

#include <mmpld.h>

#include "trrojan/camera.h"
#include "trrojan/random_sphere_generator.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/benchmark_base.h"
#include "trrojan/d3d12/graphics_pipeline_builder.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// The base class for sphere-rendering benchmarks provides handling of
    /// MMPLD and random data sets.
    /// </summary>
    class TRROJAND3D12_API sphere_benchmark_base : public benchmark_base {

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
        /// Type for inside tessellation factors.
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
        /// Finalise the instance,
        /// </summary>
        virtual ~sphere_benchmark_base(void) = default;

        /// <inheritdoc />
        virtual void optimise_order(configuration_set& inOutConfs) override;

        /// <inheritdoc />
        virtual std::vector<std::string> required_factors(void) const;

    protected:

        /// <summary>
        /// A type to express the properties of a particle data set as a
        /// bitmask.
        /// </summary>
        typedef random_sphere_generator::properties_type properties_type;

        /// <summary>
        /// The underlying bitmask type of <see cref="properties_type" />.
        /// </summary>
        typedef std::underlying_type<properties_type>::type property_mask_type;

        /// <summary>
        /// An identifier for a single shader variant.
        /// </summary>
        typedef std::uint64_t shader_id_type;

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
        /// Answer whether all bits of <paramref name="technique" /> are set
        /// in <see cref="shaderCode" />, ie whether the shader identified by
        /// <paramref namae="shaderCode" /> can be used to render the given
        /// <paramref name="technique" />.
        /// </summary>
        static inline bool is_technique(const shader_id_type shaderCode,
                const shader_id_type technique) {
            return ((shaderCode & technique) == technique);
        }

        /// <summary>
        /// Parses the data set in the given <see cref="configuration" /> as
        /// configuration of random spheres.
        /// </summary>
        /// <param name="config"></param>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        static random_sphere_generator::description parse_random_sphere_desc(
            const configuration& config, const shader_id_type shader_code);

        /// <summary>
        /// Sets the necessary shaders for the given
        /// <see cref="shader_id_type" /> in the given pipeline state builder.
        /// </summary>
        /// <param name="builder"></param>
        /// <param name="shader_id"></param>
        static void set_shaders(graphics_pipeline_builder& builder,
            const shader_id_type shader_id);

        static const property_mask_type property_float_colour;
        static const property_mask_type property_per_sphere_colour;
        static const property_mask_type property_per_sphere_intensity;
        static const property_mask_type property_per_sphere_radius;
        static const property_mask_type property_structured_resource;

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        sphere_benchmark_base(const std::string& name);

        /// <summary>
        /// Retrieves the properties of the currently loaded data set and erases
        /// all flags which are not relevant for the given shader technique.
        /// </summary>
        property_mask_type get_data_properties(const shader_id_type shader_code);

        /// <summary>
        /// Gets a <see cref="graphics_pipeline_builder" /> preconfigured for
        /// the given shader and the loaded data set.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        graphics_pipeline_builder get_pipeline_builder(
            const shader_id_type shader_code);

        /// <summary>
        /// Gets the pipeline state for the given shading technique.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12PipelineState> get_pipeline_state(
            ID3D12Device *device, const shader_id_type shader_code);

        /// <summary>
        /// Load the data set into an upload buffer and store its properties in
        /// the class.
        /// </summary>
        /// <remarks>
        /// The buffer returned is an upload buffer in read state. Callers need
        /// to copy its content to a resource that can be used as vertex buffer
        /// or structured resource buffer for rendering. The benchmark class
        /// will not keep any copy of this buffer, ie it will be in the same
        /// state as if one had called <see cref="load_data_properties" />.
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        ATL::CComPtr<ID3D12Resource> load_data(ID3D12Device *device,
            const shader_id_type shader_code, const configuration& config);

        /// <summary>
        /// Load the data set properties and input layout for the given
        /// configuration, but not the data itself.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        void load_data_properties(const shader_id_type shader_code,
            const configuration& config);


        trrojan::perspective_camera _camera;
        ATL::CComPtr<ID3D12Resource> _colour_map;
        ATL::CComPtr<ID3D12Resource> _sphere_constants;
        ATL::CComPtr<ID3D12Resource> _tessellation_constants;
        ATL::CComPtr<ID3D12Resource> _view_constants;

    private:

        typedef std::unordered_map<shader_id_type,
            graphics_pipeline_builder> pipeline_builder_map_type;
        typedef std::unordered_map<shader_id_type,
            ATL::CComPtr<ID3D12PipelineState>> pipline_state_map_type;

        pipeline_builder_map_type _builder_cache;
        properties_type _data_properties;
        std::vector<D3D12_INPUT_ELEMENT_DESC> _input_layout;
        pipline_state_map_type _pipeline_cache;

#if 0
        /// <inheritdoc />
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) override;

    private:

        /*/// <summary>
        /// A hash table for all shader sources.
        /// </summary>
        typedef std::unordered_map<shader_id_type, shader_resources>
            shader_source_map_type;

        /// <summary>
        /// A hash table for rendering techniques.
        /// </summary>
        typedef std::unordered_map<shader_id_type, rendering_technique>
            technique_map_type;*/

        /// <summary>
        /// Check whether the currently loaded data are compatible with the
        /// requirements specified in the given shader code.
        /// </summary>
        bool check_data_compatibility(const shader_id_type shaderCode);



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
#endif
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
