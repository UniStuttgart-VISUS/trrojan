// <copyright file="sphere_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <unordered_map>

#include <mmpld.h>

#include "trrojan/camera.h"
#include "trrojan/random_sphere_generator.h"

#include "trrojan/d3d12/benchmark_base.h"
#include "trrojan/d3d12/graphics_pipeline_builder.h"


/* Forward declarations. */
struct SphereConstants;
struct TessellationConstants;
struct ViewConstants;


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
        /// Gets the expected primitive toplogy for the given shader code.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        static D3D12_PRIMITIVE_TOPOLOGY get_primitive_topology(
            const shader_id_type shader_code);

        /// <summary>
        /// Gets the expected primitive toplogy for the given shader code.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        static D3D12_PRIMITIVE_TOPOLOGY_TYPE get_primitive_topology_type(
            const shader_id_type shader_code);

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
        /// Populates the tessellation constants from the given configuration.
        /// </summary>
        /// <param name="out_constants"></param>
        /// <param name="config"></param>
        static void get_tessellation_constants(
            TessellationConstants& out_constants,
            const configuration& config);

        /// <summary>
        /// Answer whether the given type of particle colour is a non-floating
        /// point per-sphere colour type.
        /// </summary>
        /// <param name="colour"></param>
        /// <returns></returns>
        static bool is_non_float_colour(const mmpld::colour_type colour);

        /// <summary>
        /// Answer whether the given MMPLD particles list uses a non-floating
        /// point per-sphere colour type.
        /// </summary>
        /// <param name="header"></param>
        /// <returns></returns>
        inline static bool is_non_float_colour(
                const mmpld::list_header& header) {
            return is_non_float_colour(header.colour_type);
        }

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
        /// Create a shader resource view for the colour map.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="handle"></param>
        void create_colour_map_view(ID3D12Device *device,
            const D3D12_CPU_DESCRIPTOR_HANDLE handle);

        /// <summary>
        /// Create constant buffer views for the constants in the upload heap.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="buffer"></param>
        /// <param name="sphere_constants"></param>
        /// <param name="tessellation_constants"></param>
        /// <param name="view_constants"></param>
        void create_constant_buffer_view(ID3D12Device *device,
            const UINT buffer,
            const D3D12_CPU_DESCRIPTOR_HANDLE sphere_constants,
            const D3D12_CPU_DESCRIPTOR_HANDLE tessellation_constants,
            const D3D12_CPU_DESCRIPTOR_HANDLE view_constants);

        /// <summary>
        /// Fits the stored bounding box to the actual data in the given MMPLD
        /// particle list.
        /// </summary>
        /// <param name="header"></param>
        /// <param name="particles"></param>
        void fit_bounding_box(const mmpld::list_header& header,
            const void *particles);

        /// <summary>
        /// Answer the size of the bounding box of the currently loaded data
        /// set.
        /// </summary>
        /// <returns></returns>
        std::array<float, 3> get_data_extents(void) const;

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
        /// Get the root signature for the given shading technique.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12RootSignature> get_root_signature(
            ID3D12Device *device, const shader_id_type shader_code);

        /// <summary>
        /// Retreives the constant buffer data for the current data properties.
        /// </summary>
        /// <param name="out_constants"></param>
        void get_sphere_constants(SphereConstants& out_constants) const;

        /// <summary>
        /// Gets the number of spheres currently loaded to the data buffer.
        /// </summary>
        /// <returns></returns>
        inline UINT get_sphere_count(void) const noexcept {
            return this->_cnt_spheres;
        }

        /// <summary>
        /// Compute the view constants from the current camera parameters.
        /// </summary>
        /// <param name="out_constants"></param>
        void get_view_constants(ViewConstants& out_constants) const;

        /// <summary>
        /// Load the data set into an upload buffer and store its properties in
        /// the class.
        /// </summary>
        /// <remarks>
        /// <para>The buffer returned is an upload buffer in read state. Callers
        /// need to copy its content to a resource that can be used as vertex
        /// buffer or structured resource buffer for rendering. The benchmark
        /// class will not keep any copy of this buffer, ie it will be in the
        /// same state as if one had called <see cref="load_data_properties" />.
        /// </para>
        /// <para>If <see cref="factor_fit_bounding_box" /> and an MMPLD file is
        /// loaded, this method will recompute the bounding box when loading it.
        /// </para>
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
        /// <remarks>
        /// <para>The <see cref="factor_fit_bounding_box" /> will not be
        /// honoured by this method, because the data need to be loaded to fit
        /// the bounding box. In turn, this method will set the bounding box
        /// from the list header in the file in any case.</para>
        /// <para>If loading an MMPLD file, the maximum radius retrieved is
        /// only correct for data sets with global radius. In any other case,
        /// the radius can only be retrieved by actually loading the data.
        /// </para>
        /// </remarks>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        void load_data_properties(const shader_id_type shader_code,
            const configuration& config);

        /// <summary>
        /// (Re-) Create graphics resource when switching the device.
        /// </summary>
        /// <param name="device"></param>
        virtual void on_device_switch(ID3D12Device *device) override;

        /// <summary>
        /// Computes the clipping planes for the current data set properties and
        /// applies them to <see cref="_camera" />.
        /// </summary>
        void set_clipping_planes(void);

        /// <summary>
        /// Determines the actual maximum radius of any sphere in the given
        /// MMPLD particle list and persists it.
        /// </summary>
        /// <remarks>
        /// This method will check the whole particle list in case the header
        /// indicates varying radii. Otherwise, the data from the header is
        /// used.
        /// </remarks>
        /// <param name="header"></param>
        /// <param name="particles"></param>
        void set_max_radius(const mmpld::list_header& header,
            const void *particles);

        /// <summary>
        /// Persists the data properties of the given random-sphere data set.
        /// </summary>
        /// <param name="desc"></param>
        void set_properties(const random_sphere_generator::description& desc);

        /// <summary>
        /// Persists the data properties of the given MMPLD particle list.
        /// </summary>
        /// <param name="header"></param>
        void set_properties(const mmpld::list_header& header);

        /// <summary>
        /// Updates the upload buffer for all constants with the most recent
        /// data.
        /// </summary>
        /// <param name="config">The configuration for which the constants are
        /// intended.</param>
        /// <param name="buffer">The index of the currently active buffer, which
        /// determines where in the constant buffer the update is performed.
        /// </param>
        void update_constants(const configuration& config, const UINT buffer);

        /// <summary>
        /// The camera used to compute the view and projection transforms.
        /// </summary>
        trrojan::perspective_camera _camera;

        /// <summary>
        /// Persistently mapped location of the global sphere description
        /// constants in the upload buffer for each frame.
        /// </summary>
        /// <remarks>
        /// <para>This variable is an array of range
        /// [0, this->pipeline_depth()[. Applications must make sure to write
        /// to the correct frame when updating the constans as the buffer is
        /// persistently mapped and other entries might be in use by the GPU.
        /// </para>
        /// <para>This variable is only valid after a call to
        /// <see cref="on_device_switch" />. Each further call to this method
        /// will also discard data stored in the previously mapped region.
        /// </para>
        /// </remarks>
        SphereConstants *_sphere_constants;

        /// <summary>
        /// Persistently mapped location of the tessellation constants in the
        /// upload buffer for each frame.
        /// </summary>
        /// <remarks>
        /// <para>This variable is an array of range
        /// [0, this->pipeline_depth()[. Applications must make sure to write
        /// to the correct frame when updating the constans as the buffer is
        /// persistently mapped and other entries might be in use by the GPU.
        /// </para>
        /// <para>This variable is only valid after a call to
        /// <see cref="on_device_switch" />. Each further call to this method
        /// will also discard data stored in the previously mapped region.
        /// </para>
        /// </remarks>
        TessellationConstants *_tessellation_constants;

        /// <summary>
        /// Persistently mapped location of the view constants in the upload
        /// buffer for each frame.
        /// </summary>
        /// <remarks>
        /// <para>This variable is an array of range
        /// [0, this->pipeline_depth()[. Applications must make sure to write
        /// to the correct frame when updating the constans as the buffer is
        /// persistently mapped and other entries might be in use by the GPU.
        /// </para>
        /// <para>This variable is only valid after a call to
        /// <see cref="on_device_switch" />. Each further call to this method
        /// will also discard data stored in the previously mapped region.
        /// </para>
        /// </remarks>
        ViewConstants *_view_constants;

    private:

        typedef std::unordered_map<shader_id_type,
            ATL::CComPtr<ID3D12PipelineState>> pipline_state_map_type;
        typedef std::unordered_map<shader_id_type,
            ATL::CComPtr<ID3D12RootSignature>> root_signature_map_type;

        glm::vec3 _bbox[2];
        UINT _cnt_spheres;
        std::array<float, 4> _colour;
        ATL::CComPtr<ID3D12Resource> _colour_map;
        ATL::CComPtr<ID3D12Resource> _constant_buffer;
        properties_type _data_properties;
        std::vector<D3D12_INPUT_ELEMENT_DESC> _input_layout;
        std::array<float, 2> _intensity_range;
        float _max_radius;
        pipline_state_map_type _pipeline_cache;
        root_signature_map_type _root_sig_cache;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
