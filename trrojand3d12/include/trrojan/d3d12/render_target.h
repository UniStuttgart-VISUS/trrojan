// <copyright file="render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <vector>

#include <Windows.h>
#include <atlbase.h>
#include <d3d12.h>

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

        render_target_base(const render_target_base&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~render_target_base(void);

        /// <summary>
        /// Queues clearing the render (and depth) target.
        /// </summary>
        /// <remarks>
        /// A device must have been set and the render target must have been
        /// resized at least once before this method can be called.
        /// </remarks>
        /// <param name="cmdList"></param>
        virtual void clear(ID3D12GraphicsCommandList *cmdList);

        /// <summary>
        /// Answer the device the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D12Device>& device(void) {
            return this->_device;
        }

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
        /// Performs cleanup operations once a frame was completed.
        /// </summary>
        /// <remarks>
        /// The default implementation transitions the render target view to
        /// present mode.
        /// </remarks>
        /// <param name="cmdList"></param>
        virtual void present(ID3D12GraphicsCommandList *cmdList);

        /// <summary>
        /// Answer the number of buffers used by the render target.
        /// </summary>
        /// <returns></returns>
        inline UINT pipeline_depth(void) const {
            return static_cast<UINT>(this->_buffers.size());
        }

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
        /// Wait until the GPU is ready to being the next frame.
        /// </summary>
        virtual void wait_for_frame(void) = 0;

        /// <summary>
        /// Inject a single into the command queue for the current frame and
        /// wait until the GPU singals it.
        /// </summary>
        void wait_for_gpu(void);

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
        D3D12_CPU_DESCRIPTOR_HANDLE current_rtv_handle(void);

        /// <summary>
        /// Wait for the GPU to finish all work and reset the buffers
        /// afterwards.
        /// </summary>
        /// <remarks>
        /// This method must be called before resizing swap chain buffers.
        /// </remarks>
        virtual void reset_buffers(void);

        /// <summary>
        /// Set <paramref name="buffers" /> as the buffers for the render target
        /// views.
        /// </summary>
        void set_buffers(
            const std::vector<ATL::CComPtr<ID3D12Resource>>& buffers,
            const UINT buffer_index = 0);

        /// <summary>
        /// Waits for the current frame to complete and sets the given value as
        /// the next one..
        /// </summary>
        /// <param name="nextFrame"></param>
        void wait_for_frame(const UINT nextFrame);

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
