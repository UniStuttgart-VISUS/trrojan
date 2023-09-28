// <copyright file="render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <Windows.h>
#include <atlbase.h>
#include <d3d12.h>

#include <winrt/windows.ui.core.h>

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/export.h"
#include "trrojan/d3d12/handle.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for production and debug render targets.
    /// </summary>
    class TRROJAND3D12_API render_target_base {

    public:

        /// <summary>
        /// Specifies the default clear colour for all render targets.
        /// </summary>
        static const std::array<float, 4> default_clear_colour;

        render_target_base(const render_target_base&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~render_target_base(void);

        /// <summary>
        /// Gets the index of the buffer in the pipeline that is currently being
        /// rendered.
        /// </summary>
        /// <remarks>
        /// Having access to this information in combination with
        /// <see cref="pipeline_depth" />allows benchmarks to overlap their work
        /// in a similar fashion.
        /// </remarks>
        /// <returns></returns>
        inline UINT buffer_index(void) const {
            return this->_buffer_index;
        }

        /// <summary>
        /// Queues clearing the render (and depth) target.
        /// </summary>
        /// <remarks>
        /// A device must have been set and the render target must have been
        /// resized at least once before this method can be called.
        /// </remarks>
        /// <param name="clear_colour"></param>
        /// <param name="cmd_list"></param>
        inline void clear(const std::array<float, 4>& clear_colour,
                ID3D12GraphicsCommandList *cmd_list) {
            this->clear(clear_colour, cmd_list, this->_buffer_index);
        }

        /// <summary>
        /// Queues clearing the specified render (and depth) target.
        /// </summary>
        /// <param name="clear_colour"></param>
        /// <param name="cmd_list"></param>
        /// <param name="frame"></param>
        virtual void clear(const std::array<float, 4>& clear_colour,
            ID3D12GraphicsCommandList *cmd_list, const UINT frame);

        /// <summary>
        /// Queues clearing the render (and depth) target to the default clear
        /// colour.
        /// </summary>
        /// <remarks>
        /// A device must have been set and the render target must have been
        /// resized at least once before this method can be called.
        /// </remarks>
        /// <param name="cmd_list"></param>
        inline void clear(ID3D12GraphicsCommandList *cmd_list) {
            this->clear(default_clear_colour, cmd_list, this->_buffer_index);
        }

        /// <summary>
        /// Queues clearing the specified render (and depth) target to the
        /// default clear colour.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="frame"></param>
        inline void clear(ID3D12GraphicsCommandList *cmd_list,
                const UINT frame) {
            this->clear(default_clear_colour, cmd_list, frame);
        }

        /// <summary>
        /// Answer the device the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D12Device>& device(void) {
            return this->_device;
        }

        /// <summary>
        /// Answer the dxgi factory the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<IDXGIFactory4>& factory(void) {
            return this->_dxgi_factory;
        }

        /// <summary>
        /// Answer the command queue the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D12CommandQueue>& command_queue(void) {
            return this->_command_queue;
        }

        /// <summary>
        /// Queue the currently active frame being transitioned to presentation
        /// state into <paramref name="cmd_list" />.
        /// </summary>
        /// <param name="cmdList"></param>
        void disable(ID3D12GraphicsCommandList *cmd_list);

        /// <summary>
        /// Queue the given <paramref name="frame" /> being transitioned to
        /// presentation state into <paramref name="cmd_list" />.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="frame"></param>
        void disable(ID3D12GraphicsCommandList *cmd_list, const UINT frame);

        /// <summary>
        /// Queues the render target as active target in the given command list.
        /// </summary>
        /// <remarks>
        /// <para>This method also sets the viewport and the scissor
        /// rectangle.</para>
        /// </remarks>
        /// <param name="cmdList"></param>
        void enable(ID3D12GraphicsCommandList *cmdList);

        /// <summary>
        /// Queues the specified buffer/frame of the render target being enabled
        /// to the given command list.
        /// </summary>
        /// <remarks>
        /// <para>This method also sets the viewport and the scissor
        /// rectangle.</para>
        /// </remarks>
        /// <param name="cmdList"></param>
        /// <param name="frame"></param>
        void enable(ID3D12GraphicsCommandList *cmdList, const UINT frame);

        /// <summary>
        /// Answer the number of buffers used by the render target.
        /// </summary>
        /// <returns></returns>
        inline UINT pipeline_depth(void) const {
            return static_cast<UINT>(this->_buffers.size());
        }

        /// <summary>
        /// Rotates the active buffer in a ring-buffer fashion and waits for the
        /// new active one to become ready.
        /// </summary>
        /// <remarks>
        /// <para>This method (the very implementation in
        /// <see cref="render_target_base" />) should only be used for offscreen
        /// targets for which the base class manages the active buffer. Targets
        /// with swap chains should actually present the swap chain and must ask
        /// the swap chain what the next buffer is and override this method to
        /// switch to the buffer index reported by the swap chain.</para>
        /// </remarks>
        /// <returns>The buffer index of the next frame.</returns>
        virtual UINT present(void);

        /// <summary>
        /// Resizes the swap chain of the render target to the given dimension.
        /// </summary>
        /// <param name="width">The new width of the render target in pixels.
        /// </param>
        /// <param name="height">The new height of the render target in pixels.
        /// </param>
        virtual void resize(const unsigned int width,
            const unsigned int height) = 0;

        /// <summary>
        /// Stage the current back buffer and save it to the given file, which
        /// of the extension determines the image codec.
        /// </summary>
        void save(const std::string& path);

        ///// <summary>
        ///// Creates an unordered access view for the back buffer, which must
        ///// have been created before.
        ///// </summary>
        ///// <param name="dst">The CPU descriptor handle that represents the
        ///// start of the heap holding the UAV.</param>
        ///// <param name="cmd_list">The command list used to transition the
        ///// resource.</param>
        //virtual void to_uav(const D3D12_CPU_DESCRIPTOR_HANDLE dst,
        //    ID3D12GraphicsCommandList *cmd_list);

        /// <summary>
        /// Enables or disables used of reversed 32-bit depth buffer.
        /// </summary>
        void use_reversed_depth_buffer(const bool isEnabled);

        /// <summary>
        /// Answer the viewport of the render target.
        /// </summary>
        inline const D3D12_VIEWPORT& viewport(void) const noexcept {
            return this->_viewport;
        }

        render_target_base& operator =(const render_target_base&) = delete;

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="device">The device to create the target on.</param>
        /// <param name="pipelineDepth">The number of buffers to be used by the
        /// render target. The default is 2, which means one front buffer and
        /// one back buffer.</param>
        render_target_base(const trrojan::device& device,
            const UINT pipelineDepth = 2);

        /// <summary>
        /// Convenience method for allocating a descriptor heap.
        /// </summary>
        /// <param name="type"></param>
        /// <param name="cnt"></param>
        /// <returns></returns>
        ATL::CComPtr<ID3D12DescriptorHeap> create_descriptor_heap(
            const D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT cnt);

        /// <summary>
        /// Allocate a descriptor heap for depth-stencil views.
        /// </summary>
        /// <param name=""></param>
        void create_dsv_heap(void);

        /// <summary>
        /// Allocate a descriptor heap for render target views.
        /// </summary>
        void create_rtv_heap(void);

        /// <summary>
        /// Allocate a swap chain for the pipeline depth of the render target
        /// and associated with the given window.
        /// </summary>
        /// <remarks>
        /// This method is a convenience method for subclasses to create a
        /// compatible swap chain.
        /// </remarks>
        /// <param name="hWnd"></param>
        /// <returns></returns>
        ATL::CComPtr<IDXGISwapChain3> create_swap_chain(HWND hWnd);

#ifdef _UWP
        /// <summary>
        /// Allocate a swap chain for the pipeline depth of the render target
        /// and associated with the given window for UWP.
        /// </summary>
        /// <remarks>
        /// This method is a convenience method for subclasses to create a
        /// compatible swap chain.
        /// </remarks>
        /// <param name="width"></param>
        /// <param name="height"></param>
        /// <param name="window"></param>
        /// <returns></returns>
        ATL::CComPtr<IDXGISwapChain3> create_swap_chain(UINT width, UINT height, 
            winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> window);
#endif // _UWP

        /// <summary>
        /// Answer the current buffer.
        /// </summary>
        /// <returns></returns>
        ATL::CComPtr<ID3D12Resource> current_buffer(void);

        /// <summary>
        /// Answer the CPU handle of the DSV of the current frame.
        /// </summary>
        /// <returns></returns>
        inline D3D12_CPU_DESCRIPTOR_HANDLE current_dsv_handle(void) {
            return this->_dsv_heap->GetCPUDescriptorHandleForHeapStart();
        }

        /// <summary>
        /// Answer the CPU handle of the RTV of the current frame.
        /// </summary>
        /// <returns></returns>
        inline D3D12_CPU_DESCRIPTOR_HANDLE current_rtv_handle(void) {
            return this->rtv_handle(this->_buffer_index);
        }

        /// <summary>
        /// Answer the CPU handle of the DSV of the given frame.
        /// </summary>
        /// <param name="frame"></param>
        /// <returns></returns>
        inline D3D12_CPU_DESCRIPTOR_HANDLE dsv_handle(const UINT frame) {
            return this->_dsv_heap->GetCPUDescriptorHandleForHeapStart();
        }

        /// <summary>
        /// Wait for the GPU to finish all work and reset the buffers
        /// afterwards.
        /// </summary>
        /// <remarks>
        /// This method must be called before resizing swap chain buffers.
        /// </remarks>
        virtual void reset_buffers(void);

        /// <summary>
        /// Answer the CPU handle of the RTV of the specified frame.
        /// </summary>
        /// <param name="frame"></param>
        /// <returns></returns>
        D3D12_CPU_DESCRIPTOR_HANDLE rtv_handle(const UINT frame);

        /// <summary>
        /// Set <paramref name="buffers" /> as the buffers for the render target
        /// views.
        /// </summary>
        void set_buffers(
            const std::vector<ATL::CComPtr<ID3D12Resource>>& buffers,
            const UINT buffer_index = 0);

        /// <summary>
        /// Waits for the specified butter becoming ready and then sets this
        /// buffer the active one.
        /// </summary>
        /// <remarks>
        /// This method is intended for subclasses that use swap chains and
        /// need to override the parameterless overload of the method.
        /// </remarks>
        /// <param name="next_buffer">The index of the next buffer. The swap
        /// chain should provide callers with this value.</param>
        void switch_buffer(const UINT next_buffer);

        /// <summary>
        /// Block the calling thread until the GPU completed all pending work.
        /// </summary>
        void wait_for_gpu(void);

        /// <summary>
        /// Clear value for the depth buffer.
        /// </summary>
        float _depth_clear;

        /// <summary>
        /// A depth-stencil state that is being activated with the target.
        /// </summary>
        //ATL::CComPtr<ID3D11DepthStencilState> _dss;

        /// <summary>
        /// The viewport matching the size of the render target.
        /// </summary>
        D3D12_VIEWPORT _viewport;

    private:

        /// <summary>
        /// Identifies the back buffer currently used for rendering.
        /// </summary>
        UINT _buffer_index;

        /// <summary>
        /// Holds the resources of the render target buffers.
        /// </summary>
        std::vector<ATL::CComPtr<ID3D12Resource>> _buffers;

        /// <summary>
        /// The direct command queue assoicated with <see cref="_device" />.
        /// </summary>
        ATL::CComPtr<ID3D12CommandQueue> _command_queue;

        /// <summary>
        /// The depth buffer of the render target.
        /// </summary>
        /// <remarks>
        /// In contrast to D3D11, the resource view is not an object any more
        /// that keeps the resource alive. Therefore, the render target must
        /// keep a reference to the depth buffer.
        /// </remarks>
        ATL::CComPtr<ID3D12Resource> _depth_buffer;

        /// <summary>
        /// The device the render target lives on.
        /// </summary>
        ATL::CComPtr<ID3D12Device> _device;

        /// <summary>
        /// The heap holding the depth-stencil views.
        /// </summary>
        ATL::CComPtr<ID3D12DescriptorHeap> _dsv_heap;

        /// <summary>
        /// The DXGI factory from which the underlying device was obtained.
        /// </summary>
        ATL::CComPtr<IDXGIFactory4> _dxgi_factory;

        /// <summary>
        /// A fence for waiting on the GPU to complete a task.
        /// </summary>
        ATL::CComPtr<ID3D12Fence> _fence;

        /// <summary>
        /// An event for blocking the calling thread until a fence in the GPU
        /// command stream was reached.
        /// </summary>
        handle<> _fence_event;

        /// <summary>
        /// The values of the fence for each frame.
        /// </summary>
        std::vector<UINT64> _fence_values;

        /// <summary>
        /// The size of the render target view descriptor.
        /// </summary>
        UINT _rtv_descriptor_size;

        /// <summary>
        /// The heap holding the render target views.
        /// </summary>
        ATL::CComPtr<ID3D12DescriptorHeap> _rtv_heap;

        /// <summary>
        /// A staging texture for saving the back buffer to disk.
        /// </summary>
        ATL::CComPtr<ID3D12Resource> _staging_texture;
    };


    /// <summary>
    /// Pointer to a generic render target.
    /// </summary>
    typedef std::shared_ptr<render_target_base> render_target;
}
}
