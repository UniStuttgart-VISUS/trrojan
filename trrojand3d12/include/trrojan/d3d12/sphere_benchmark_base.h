// <copyright file="sphere_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <unordered_map>

#include "trrojan/camera.h"

#include "trrojan/d3d12/benchmark_base.h"
#include "trrojan/d3d12/graphics_pipeline_builder.h"
#include "trrojan/d3d12/sphere_data.h"
#include "trrojan/d3d12/sphere_rendering_configuration.h"



namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// The base class for sphere-rendering benchmarks provides handling of
    /// MMPLD and random data sets.
    /// </summary>
    class TRROJAND3D12_API sphere_benchmark_base : public benchmark_base {

    public:

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
        typedef sphere_data::descriptor_table_type descriptor_table_type;

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
        /// Gets the number of vertices per instance and the number of instances
        /// to emit for the given number of spheres and the given shader ID.
        /// </summary>
        /// <param name="shader_code">The code identifying the rendering
        /// technique.</param>
        /// <param name="spheres">The number of spheres to be rendered.</param>
        /// <returns>The vertex count per instance in
        /// <see cref="std::pair::first" /> and the number of instances in
        /// <see cref="std::pair::second" />.</returns>
        static std::pair<UINT, UINT> get_draw_count(
            const shader_id_type shader_code, const std::size_t spheres);

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
        /// Answer whether any bit of <paramref name="technique" /> is set
        /// in <see cref="shader_code" />.
        /// </summary>
        static inline bool is_any_technique(const shader_id_type shader_code,
                const shader_id_type technique) {
            return ((shader_code & technique) != 0);
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

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        sphere_benchmark_base(const std::string& name);

        /// <summary>
        /// Clear the data loaded if the given changes in parameters invalidate
        /// them.
        /// </summary>
        /// <returns><c>true</c> if the data have been cleared, <c>false</c>
        /// otherwise.</returns>
        bool clear_stale_data(const std::vector<std::string>& changed);

        /// <summary>
        /// Updates the field of view and the aspect ratio of the current
        /// viewport and then computes the clipping planes for the current data
        /// set properties and applies them to <see cref="_camera" />.
        /// </summary>
        /// <param name="config">The configuration from which the manoeuvre is
        /// retrieved from.</param>
        /// <param name="fovy">The field of view on the y-axis in degrees, which
        /// will be applied on the camera. This parameter defaults to 60
        /// degrees.</param>
        void configure_camera(const configuration& config,
            const float fovy = 60.0f);

        /// <summary>
        /// Determine how many descriptor tables are required for the given
        /// rendering technique.
        /// </summary>
        /// <param name="shader_code"></param>
        /// <param name="include_root"></param>
        /// <returns></returns>
        virtual UINT count_descriptor_tables(const shader_id_type shader_code,
            const bool include_root) const;

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
        /// the specified rendering technique as computed by
        /// <see cref="count_descriptor_tables" />.
        /// </summary>
        /// <para>Descriptor heaps are allocated for all frames of the pipeline.
        /// </para>
        /// <para>Derived classes that want to influence this method should
        /// override <see cref="count_descriptor_tables" />.</para>
        /// </remarks>
        /// <remarks>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        virtual void create_descriptor_heaps(ID3D12Device *device,
            const shader_id_type shader_code);

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
        /// Gets the GPU-virtual address for the sphere constants of the given
        /// frame/buffer.
        /// </summary>
        /// <param name="buffer"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS get_sphere_constants(const UINT buffer) const;

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
        /// (Re-) Create graphics resource when switching the device.
        /// </summary>
        /// <param name="device"></param>
        virtual void on_device_switch(device& device) override;

        /// <summary>
        /// Creates a host-memory description holding all descriptors that need
        /// to be set for the given shader.
        /// </summary>
        /// <remarks>
        /// <para>This method will check all potential shader resource views
        /// that are available for sphere renderers and add them to the returned
        /// descriptor table if necessary. The return value holds only the
        /// descriptors that are expected by the shaders described by
        /// <paramref name="shader_code" />.</para>
        /// <para>The descriptors created are not applied on the device, but
        /// must be passed to a command list before issuing draw calls.</para>
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        /// <param name="frame"></param>
        /// <param name="first_descriptor">An offset for the first descriptor to
        /// be used. This can be used for application cases where there are
        /// multiple descriptor tables per frame in the same heap.</param>
        /// <param name="data">If non-<c>nullptr</c>, overwrites the data set in
        /// <see cref="_data" />. This is typically used in combination with
        /// <see cref="first_data_element" /> and <paramref name="cnt_data" />
        /// in order to overwrite the default <see cref="_data" /> field
        /// managing the data set in this base class.</param>
        /// <param name="first_data_element">If non-zero, overwrites the offset
        /// of the data to be used.</param>
        /// <param name="cnt_data">If non-zero, overwrites the size of the data
        /// being mapped.</param>
        /// <returns>The descriptor heap that was used to set the shader
        /// resource views and constant buffers.</returns>
        virtual descriptor_table_type set_descriptors(ID3D12Device *device,
            const shader_id_type shader_code,
            const UINT frame,
            const UINT64 first_descriptor = 0,
            ID3D12Resource *data = nullptr,
            const UINT64 first_data_element = 0,
            const UINT cnt_data = 0);

        /// <summary>
        /// Creates the host-memory description of the descriptors required for
        /// the given shader using the descriptor table of the current frame.
        /// </summary>
        /// <remarks>
        /// <para>As descriptor tables are allocated on a descriptor heap and
        /// there are separate descriptor heaps for each frame in the pipeline,
        /// the descriptors must be set for every frame individually. This
        /// overload does this for the currently active frame rather than for
        /// a user-specified one.</para>
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="shader_code"></param>
        inline descriptor_table_type set_descriptors(ID3D12Device *device,
                const shader_id_type shader_code) {
            return this->set_descriptors(device, shader_code,
                this->buffer_index());
        }

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
        void update_constants(const sphere_rendering_configuration& config,
            const UINT buffer);

        /// <summary>
        /// The camera used to compute the view and projection transforms.
        /// </summary>
        trrojan::perspective_camera _camera;

        /// <summary>
        /// Manages the data set and its properties.
        /// </summary>
        sphere_data _data;

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

        UINT _cnt_descriptor_tables;
        ATL::CComPtr<ID3D12Resource> _colour_map;
        ATL::CComPtr<ID3D12Resource> _constant_buffer;
        pipline_state_map_type _pipeline_cache;
        root_signature_map_type _root_sig_cache;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
