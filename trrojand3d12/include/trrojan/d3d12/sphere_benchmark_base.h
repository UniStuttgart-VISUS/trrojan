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
        /// A descriptor table repesented as a descriptor heap and offsets to
        /// the entries in that heap.
        /// </summary>
        typedef std::pair<ATL::CComPtr<ID3D12DescriptorHeap>,
            std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> descriptor_table_type;

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
        /// <remarks>
        /// This method only retrieves the base code of the technique with the
        /// specified name. The code returned does not contain any flags
        /// modifying the behaviour, which can be specified in the
        /// configuration, nor does it contain any data-dependent features.
        /// </remarks>
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
        /// later. Note that some flags in the returned code are set
        /// speculatively and might need to be erased if they are not relevant
        /// for the data being visualised.
        /// </remarks>
        /// <param name="config"></param>
        /// <returns></returns>
        static shader_id_type get_shader_id(const configuration& config);

        /// <summary>
        /// Computes the final shader code from what has been requested by the
        /// configuration and the data that have been loaded.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <param name="data_code"></param>
        /// <returns></returns>
        static shader_id_type get_shader_id(shader_id_type shader_code,
            property_mask_type data_code);

        /// <summary>
        /// Populates the tessellation constants from the given configuration.
        /// </summary>
        /// <param name="out_constants"></param>
        /// <param name="config"></param>
        static void get_tessellation_constants(
            TessellationConstants& out_constants,
            const configuration& config);

        /// <summary>
        /// Answer whether any bit of <paramref name="technique" /> is set
        /// in <see cref="shader_code" />.
        /// </summary>
        static inline bool is_any_technique(const shader_id_type shader_code,
                const shader_id_type technique) {
            return ((shader_code & technique) != 0);
        }

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
        /// Apply the given descriptor table as root descriptors in the given
        /// command list.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="descriptors"></param>
        static void set_descriptors(ID3D12GraphicsCommandList *cmd_list,
            const descriptor_table_type& descriptors);

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
        /// Updates the field of view and the aspect ratio of the current view port
        /// and then computes the clipping planes for the current data set
        /// properties and applies them to <see cref="_camera" />.
        /// </summary>
        /// <param name="config">The configuration from which the manoeuvre is
        /// retrieved from.</param>
        /// <param name="fovy">The field of view on the y-axis in degrees, which
        /// will be applied on the camera. This parameter defaults to 60
        /// degrees.</param>
        void configure_camera(const configuration& config,
            const float fovy = 60.0f);

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
        /// <param name="view_constants"></param>
        /// <param name="tessellation_constants"></param>
        void create_constant_buffer_view(ID3D12Device *device,
            const UINT buffer,
            const D3D12_CPU_DESCRIPTOR_HANDLE sphere_constants,
            const D3D12_CPU_DESCRIPTOR_HANDLE view_constants,
            const D3D12_CPU_DESCRIPTOR_HANDLE tessellation_constants);

        using benchmark_base::create_descriptor_heaps;

        /// <summary>
        /// Change the descriptor heaps of the benchmark to match the needs of
        /// the specified rendering technique.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        void create_descriptor_heaps(ID3D12Device *device,
            const shader_id_type shader_code);

        /// <summary>
        /// Fits the stored bounding box to the actual data in the given MMPLD
        /// particle list.
        /// </summary>
        /// <param name="header"></param>
        /// <param name="particles"></param>
        void fit_bounding_box(const mmpld::list_header& header,
            const void *particles);

        /// <summary>
        /// Gets the end point of the bounding box.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        inline const glm::vec3& get_bbox_end(void) const noexcept {
            return this->_bbox[1];
        }

        /// <summary>
        /// Gets the start point of the bounding box.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        inline const glm::vec3& get_bbox_start(void) const noexcept {
            return this->_bbox[0];
        }

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
        /// Gets the GPU-virtual address for the sphere constants of the given
        /// frame/buffer.
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS get_sphere_constants(const UINT buffer) const;

        /// <summary>
        /// Gets the number of spheres currently loaded to the data buffer.
        /// </summary>
        /// <returns></returns>
        inline UINT get_sphere_count(void) const noexcept {
            return this->_cnt_spheres;
        }

        /// <summary>
        /// Gets the stride of the currently loaded data in bytes.
        /// </summary>
        /// <returns>The stride of the data loaded.</returns>
        inline UINT get_stride(void) const noexcept {
            return this->_stride;
        }

        /// <summary>
        /// Gets the GPU-virtual address for the tessellation constants of the
        /// given frame/buffer.
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS get_tessellation_constants(
            const UINT buffer) const;

        /// <summary>
        /// Compute the view constants from the current camera parameters.
        /// </summary>
        /// <param name="out_constants"></param>
        void get_view_constants(ViewConstants& out_constants) const;

        /// <summary>
        /// Gets the GPU-virtual address for the view/camera constants of the
        /// given frame/buffer.
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS get_view_constants(const UINT buffer) const;

        /// <summary>
        /// Load the data set into an immutable buffer and transition this
        /// buffer for use as shader resource.
        /// </summary>
        /// <remarks>
        /// <para>The buffer returned is an immutable buffer in
        /// <paramref name="state" /> after <paramref name="cmd_list" /> was
        /// executed. Callers need to run the command list before using the data
        /// set. The benchmark class will keep any copy of this buffer for
        /// creating the appropriate shader resource views or input assembler
        /// bindings.
        /// </para>
        /// <para>If <see cref="factor_fit_bounding_box" /> and an MMPLD file is
        /// loaded, this method will recompute the bounding box when loading it.
        /// </para>
        /// </remarks>
        /// <param name="cmd_list"></param>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        /// <returns>A pointer the upload buffer. Callers need to keep this
        /// buffer alive until they have executed <paramref name="cmd_list" />.
        /// </returns>
        ATL::CComPtr<ID3D12Resource> load_data(
            ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code,
            const configuration& config,
            const D3D12_RESOURCE_STATES state);

        /// <summary>
        /// Load the data set into the memory provided by
        /// <paramref name="allocator" />.
        /// </summary>
        /// <param name="allocator"></param>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        /// <returns>The size, in bytes, of the data buffer. This is the same
        /// value that is also passed to <paramref name="allocator" />.</returns>
        UINT64 load_data(std::function<void *(const UINT64)> allocator,
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
        virtual void on_device_switch(device& device) override;

        /// <summary>
        /// Applies the descriptors for the current data set and camera
        /// properties in the given command list.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        /// <param name="frame"></param>
        /// <returns>The descriptor heap that was used to set the shader
        /// resource views and constant buffers.</returns>
        virtual descriptor_table_type set_descriptors(ID3D12Device *device,
            const shader_id_type shader_code, const UINT frame);

        /// <summary>
        /// Applies the descriptors for the current data set and camera
        /// properties using the buffers for the current frame.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        inline descriptor_table_type set_descriptors(ID3D12Device *device,
                const shader_id_type shader_code) {
            return this->set_descriptors(device, shader_code,
                this->buffer_index());
        }

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
        /// Sets the vertex buffer holding the data in the given command list if
        /// the given <paramref name="shader_code "/> demands a vertex buffer.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="shader_code"></param>
        virtual void set_vertex_buffer(ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code);

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
        /// Holds a pointer to the buffer filled by <see cref="load_data" />.
        /// </summary>
        ATL::CComPtr<ID3D12Resource> _data;

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
        UINT _cnt_descriptor_tables;
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
        UINT _stride;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
