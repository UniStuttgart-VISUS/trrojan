// <copyright file="benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <d3d12.h>

#include "trrojan/graphics_benchmark_base.h"

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/render_target.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for D3D12 benchmarks.
    /// </summary>
    class TRROJAND3D12_API benchmark_base
            : public trrojan::graphics_benchmark_base {

    public:

        typedef trrojan::graphics_benchmark_base::manoeuvre_step_type
            manoeuvre_step_type;

        typedef trrojan::graphics_benchmark_base::manoeuvre_type manoeuvre_type;

        typedef trrojan::graphics_benchmark_base::point_type point_type;

        typedef trrojan::graphics_benchmark_base::viewport_type viewport_type;

        /// <summary>
        /// Boolean factor enabling the debug view.
        /// </summary>
        static const std::string factor_debug_view;

        /// <summary>
        /// Boolean factor enabling output of the frame buffer to disk.
        /// </summary>
        static const std::string factor_save_view;

        virtual ~benchmark_base(void);

        virtual bool can_run(trrojan::environment env,
            trrojan::device device) const noexcept override;

        virtual trrojan::result run(const configuration& c) override;

    protected:

        typedef std::vector<ATL::CComPtr<ID3D12CommandAllocator>>
            command_allocator_list;

        typedef ATL::CComPtr<ID3D12GraphicsCommandList> graphics_command_list;

        /// <summary>
        /// In-place sorts <paramref name="times" /> and computes the median.
        /// </summary>
        /// <param name="times"></param>
        /// <returns></returns>
        gpu_timer::millis_type calc_median(
            std::vector<gpu_timer::millis_type>& times);

        /// <summary>
        /// Appends <paramref name="cnt" /> command allocators of the specified
        /// <paramref name="type" /> on the given <paramref name="device" /> to
        /// the given list.
        /// </summary>
        /// <param name="dst"></param>
        /// <param name="device"></param>
        /// <param name="type"></param>
        /// <param name="cnt"></param>
        static void create_command_allocators(command_allocator_list& dst,
            ID3D12Device *device, const D3D12_COMMAND_LIST_TYPE type,
            const std::size_t cnt);

        /// <summary>
        /// Create a command list using the <paramref name="frame" />th
        /// allocator in <paramref name="allocators" />.
        /// </summary>
        /// <param name="allocators"></param>
        /// <param name="type"></param>
        /// <param name="frame"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        static ATL::CComPtr<ID3D12CommandList> create_command_list(
            const command_allocator_list& allocators,
            const D3D12_COMMAND_LIST_TYPE type,
            const std::size_t frame,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Resolve the actual path where a given CSO shader is located.
        /// </summary>
        /// <param name="file_name">The relative path of the shader file.
        /// </param>
        /// <returns>The absolute path of the compiled shader object.</returns>
        static std::string resolve_shader_path(const std::string& file_name);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name"></param>
        benchmark_base(const std::string& name);

        /// <summary>
        /// Gets the index of the back buffer we are currently rendering to.
        /// </summary>
        /// <remarks>
        /// Bechmark implementations can use this information to implement their
        /// own pipelining for instance for constant buffers.
        /// </remarks>
        /// <returns></returns>
        inline UINT buffer_index(void) const {
            return (this->_render_target != nullptr)
                ? this->_render_target->buffer_index()
                : 0;
        }

        /// <summary>
        /// Schedules clearing the render target to the given command list.
        /// </summary>
        /// <param name="cmd_list"></param>
        inline void clear_target(ID3D12GraphicsCommandList *cmd_list) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->clear(cmd_list);
        }

        inline void clear_target(ID3D12GraphicsCommandList *cmd_list,
                const UINT frame) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->clear(cmd_list, frame);
        }

        /// <summary>
        /// Schedules clearing the render target to the given command list.
        /// </summary>
        /// <param name="clear_colour"></param>
        /// <param name="cmd_list"></param>
        inline void clear_target(const std::array<float, 4>& clear_colour,
                ID3D12GraphicsCommandList *cmd_list) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->clear(clear_colour, cmd_list);
        }

        inline void clear_target(const std::array<float, 4>& clear_colour,
                ID3D12GraphicsCommandList *cmd_list, const UINT frame) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->clear(clear_colour, cmd_list, frame);
        }

        /// <summary>
        /// Copy the contents of <paramref name="source" /> into the specified
        /// back buffer of the render target, which must have been enabled using
        /// <see cref="D3D12_RESOURCE_STATE_COPY_DEST" /> as rendering state.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="source"></param>
        /// <param name="frame"></param>
        inline void copy_to_target(ID3D12GraphicsCommandList *cmd_list,
                ID3D12Resource *source, const UINT frame) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            assert(source != nullptr);
            this->_render_target->copy_from(cmd_list, source, frame);
        }

        /// <summary>
        /// Creates a shader resource view for a buffer.
        /// </summary>
        /// <param name="resource"></param>
        /// <param name="first_element"></param>
        /// <param name="cnt"></param>
        /// <param name="stride"></param>
        /// <param name="descriptor"></param>
        void create_buffer_resource_view(ID3D12Resource *resource,
            const UINT64 first_element, const UINT cnt, const UINT stride,
            const D3D12_CPU_DESCRIPTOR_HANDLE descriptor);

        /// <summary>
        /// Create a new command bundle using the specified allocator.
        /// </summary>
        /// <param name="allocator"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12GraphicsCommandList> create_command_bundle(
            const std::size_t allocator,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Creates a new command list of the specified type.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="frame"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12CommandList> create_command_list(
            const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Allocate generic descriptor heaps (one for each frame) into the
        /// <see cref="_descriptor_heaps" /> member that can be used for constant
        /// buffers, shader resource views and all other kinds of resource
        /// views.
        /// </summary>
        /// <remarks>
        /// <para>If <see cref="_descriptor_heaps" /> is set,
        /// <see cref="on_device_switch" /> will automatically reallocate a
        /// similar heaps on the new device.</para>
        /// <para>Subclasses can use this method for convenience of allocate the
        /// required descriptor heaps by themselves. If these are stored in
        /// <see cref="_descriptor_heaps" />, they will also be transferred to
        /// new devices in <see cref="on_device_switch" />.</para>
        /// </remarks>
        /// <param name="device">The device to create the heap on.</param>
        /// <param name="cnt">The number of descriptors in the heap.</param>
        void create_descriptor_heaps(ID3D12Device *device, const UINT cnt);

        /// <summary>
        /// Allocate specific descriptor heaps (one of the specified type of
        /// each frame) into the <see cref="_descriptor_heaps" /> member.
        /// </summary>
        /// <remarks>
        /// <para>The heaps for each frame are stored contiguously, ie for
        /// each description in <paramref name="descs" />, all heaps for
        /// the first frame are stored in <see cref="_descriptor_heaps" />,
        /// then all for the second frame, etc.</para>
        /// <para>If <see cref="_descriptor_heaps" /> is set,
        /// <see cref="on_device_switch" /> will automatically reallocate a
        /// similar heaps on the new device.</para>
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="descs"></param>
        void create_descriptor_heaps(ID3D12Device *device,
            const std::vector<D3D12_DESCRIPTOR_HEAP_DESC>& descs);

        /// <summary>
        /// Creates a new command list of the specified type and casts it to a
        /// <see cref="ID3D12GraphicsCommandList" />.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="frame"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        graphics_command_list create_graphics_command_list(
            const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Creates a new direct graphics command list using the allocator for
        /// the specified frame.
        /// </summary>
        /// <param name="frame"></param>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        graphics_command_list create_graphics_command_list(
            const std::size_t frame,
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Creates a new direct graphics command list for the currently active
        /// buffer/frame.
        /// </summary>
        /// <param name="initial_state"></param>
        /// <returns></returns>
        graphics_command_list create_graphics_command_list(
            ID3D12PipelineState *initial_state = nullptr);

        /// <summary>
        /// Creates a new direct graphics command list for the currently active
        /// buffer/frame if any of the specified <paramref name="resources" />
        /// is <c>nullptr</c>.
        /// </summary>
        /// <typeparam name="TResource"></typeparam>
        /// <param name="resources"></param>
        /// <returns></returns>
        template<class... TResource>
        graphics_command_list create_graphics_command_list_for(
            TResource&&... resources);

        /// <summary>
        /// Queues the current render target to be disabled in the given command
        /// list.
        /// </summary>
        inline void disable_target(ID3D12GraphicsCommandList *cmd_list,
                const D3D12_RESOURCE_STATES render_state
                = D3D12_RESOURCE_STATE_RENDER_TARGET) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->disable(cmd_list, render_state);
        }

        /// <summary>
        /// Queues the current render target to be disabled in the given command
        /// list.
        /// </summary>
        inline void disable_target(ID3D12GraphicsCommandList *cmd_list,
                const UINT frame, const D3D12_RESOURCE_STATES render_state
                = D3D12_RESOURCE_STATE_RENDER_TARGET) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->disable(cmd_list, frame, render_state);
        }

        /// <summary>
        /// Schedule enabling the current render target to the given command
        /// list.
        /// </summary>
        /// <remarks>
        /// This operation will (i) set the render target view, (ii) set the
        /// viewport to the dimension of the render target and (iii) set the
        /// scissor rectangle to include the whole viewport.
        /// </remarks>
        inline void enable_target(ID3D12GraphicsCommandList *cmd_list,
                const D3D12_RESOURCE_STATES render_state
                = D3D12_RESOURCE_STATE_RENDER_TARGET) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->enable(cmd_list, render_state);
        }

        /// <summary>
        /// Schedule enabling the specified frame of the current render target
        /// to the given command list.
        /// </summary>
        /// <remarks>
        /// This method behaves as the other overload of <see cref="enable" />,
        /// but it enables to preparing command lists for all frames in case
        /// the draw calls do not change over time.
        /// </remarks>
        inline void enable_target(ID3D12GraphicsCommandList *cmd_list,
                const UINT frame, const D3D12_RESOURCE_STATES render_state
                = D3D12_RESOURCE_STATE_RENDER_TARGET) {
            assert(this->_render_target != nullptr);
            assert(cmd_list != nullptr);
            this->_render_target->enable(cmd_list, frame, render_state);
        }

        /// <summary>
        /// (Re-) Create graphics resource when switching the device.
        /// </summary>
        /// <remarks>
        /// <para>Subclassses perfoming their own resource handling should call
        /// their parent class' implementation to make sure that all resources
        /// switches are handled.</para>
        /// </remarks>
        /// <param name="device"></param>
        virtual void on_device_switch(device& device);

        /// <summary>
        /// Performs the actual test on behalf of the <see cref="run" /> method.
        /// </summary>
        /// <remarks>
        /// Implementors must enable the render target before drawing the final
        /// image, the base class will not enable any target before calling this
        /// method!
        /// </remarks>
        /// <param name="device">The device to use. It is guaranteed that the
        /// device is obtained from <paramref name="config" />.</param>
        /// <param name="config">The configuration to run.</param>
        /// <param name="power_collector">The power data collector from
        /// <pararmef name="config" /> if there is any.</param>
        /// <param name="changed">The names of the factors that have been
        /// changed since the last test run.</param>
        /// <returns>The test results.</returns>
        virtual trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed) = 0;

        /// <summary>
        /// Answer the number of buffers/frames in the pipeline.
        /// </summary>
        /// <returns>The number of buffers used by the render target.</returns>
        inline UINT pipeline_depth(void) const {
            return this->_render_target->pipeline_depth();
        }

        /// <summary>
        /// Reset the direct command allocator at <paramref name="frame" /> and
        /// the given command list.
        /// </summary>
        /// <remarks>
        /// Command allocators and command lists can only be reset if the GPU
        /// finished executing the list. Callers must make sure using a fence
        /// that this is the case for <paramref name="cmd_list" />.
        /// </remarks>
        /// <param name="cmd_list"></param>
        /// <param name="frame"></param>
        /// <param name="initial_state"></param>
        void reset_command_list(ID3D12GraphicsCommandList *cmd_list,
            const UINT frame,
            ID3D12PipelineState *initial_state = nullptr) const;

        void reset_command_list(ID3D12GraphicsCommandList *cmd_list,
            ID3D12PipelineState *initial_state = nullptr) const;

        /// <summary>
        /// Swap the presentation buffer and answer the next one to write to.
        /// </summary>
        /// <remarks>
        /// The render target will block the calling thread if the previous
        /// rendering on the new buffer index (the one being returned) has not
        /// yet completed.
        /// </remarks>
        /// <returns>The index of the next buffer/frame to write to.</returns>
        inline UINT present_target(void) {
            assert(this->_render_target != nullptr);
            return this->_render_target->present();
        }

        void save_target(const char *path = nullptr);

        /// <summary>
        /// Applies the aspect ratio of the current viewport to the camera.
        /// </summary>
        /// <param name="camera"></param>
        void set_aspect_from_viewport(camera& camera);

        /// <summary>
        /// Gets the viewport of the active render target.
        /// </summary>
        /// <returns></returns>
        inline const D3D12_VIEWPORT& viewport(void) const noexcept {
            assert(this->_render_target != nullptr);
            return this->_render_target->viewport();
        }

        /// <summary>
        /// Stores command allocators for bundles.
        /// </summary>
        command_allocator_list _bundle_allocators;

        /// <summary>
        /// Stores a command allocator for each frame in the pipeline.
        /// </summary>
        command_allocator_list _compute_cmd_allocators;

        /// <summary>
        /// Stores a command allocator for each frame in the pipeline.
        /// </summary>
        command_allocator_list _copy_cmd_allocators;

        /// <summary>
        /// Stores a descriptor heap for each frame in the pipeline.
        /// </summary>
        /// <remarks>
        /// <see cref="create_descriptor_heap" /> can be used to fill this
        /// field conveniently.
        /// </remarks>
        std::vector<ATL::CComPtr<ID3D12DescriptorHeap>> _descriptor_heaps;

        /// <summary>
        /// Stores a command allocator for each frame in the pipeline.
        /// </summary>
        /// <remarks>
        /// The base class will make sure that at least one direct command
        /// allocator for each frame in the pipeline is created in
        /// <see cref="on_device_switch" />.
        /// </remarks>
        command_allocator_list _direct_cmd_allocators;

    private:

        typedef trrojan::benchmark_base base;

        render_target _debug_target;
        render_target _render_target;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */


#include "trrojan/d3d12/benchmark_base.inl"
