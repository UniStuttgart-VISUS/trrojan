// <copyright file="render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/render_target.h"

#include <cassert>

#include "trrojan/image_helper.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"


/*
 * trrojan::d3d12::render_target_base::~render_target_base
 */
trrojan::d3d12::render_target_base::~render_target_base(void) {
    this->wait_for_gpu();
}


/*
 * trrojan::d3d12::render_target_base::clear
 */
void trrojan::d3d12::render_target_base::clear(ID3D12GraphicsCommandList *cmdList) {
    assert(cmdList != nullptr);
    static const FLOAT CLEAR_COLOUR[] = { 0.0f, 0.0f, 0.0f, 0.0f }; // TODO

    {
        auto handle = this->current_dsv_handle();
        cmdList->ClearDepthStencilView(handle, D3D12_CLEAR_FLAG_DEPTH,
            this->_depth_clear, 0, 0, nullptr);
    }

    {
        auto handle = this->current_rtv_handle();
        cmdList->ClearRenderTargetView(handle, CLEAR_COLOUR, 0, nullptr);
    }
}


/*
 * trrojan::d3d12::render_target_base::enable
 */
void trrojan::d3d12::render_target_base::enable(
        ID3D12GraphicsCommandList *cmdList) {
    assert(cmdList != nullptr);

    cmdList->RSSetViewports(1, &this->_viewport);

    {
        D3D12_RECT rect;
        rect.left = rect.right = static_cast<LONG>(this->_viewport.TopLeftX);
        rect.top = rect.bottom = static_cast<LONG>(this->_viewport.TopLeftY);
        rect.right += static_cast<LONG>(this->_viewport.Width);
        rect.bottom += static_cast<LONG>(this->_viewport.Height);

        cmdList->RSSetScissorRects(1, &rect);
    }

    transition_subresource(cmdList, this->_buffers[this->_buffer_index], 0,
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    {
        auto hDsv = this->current_dsv_handle();
        auto hRtv = this->current_rtv_handle();

        cmdList->OMSetRenderTargets(1, &hRtv, FALSE, &hDsv);
    }

    //this->_device_context->OMSetDepthStencilState(this->_dss.p, 0);
    //this->_device_context->OMSetRenderTargets(1, &this->_rtv.p, this->_dsv.p);
}


/*
 * trrojan::d3d12::render_target_base::present
 */
void trrojan::d3d12::render_target_base::present(
        ID3D12GraphicsCommandList *cmdList) {
    assert(cmdList != nullptr);
    transition_subresource(cmdList, this->_buffers[this->_buffer_index], 0,
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    this->_buffer_index = ++this->_buffer_index % this->pipeline_depth();
}


/*
 * trrojan::d3d12::render_target_base::save
 */
void trrojan::d3d12::render_target_base::save(const std::string& path) {
    if (this->_buffers.front() != nullptr) {
        throw "TODO";
        //if (this->_staging_texture == nullptr) {
        //    auto desc = this->_buffers.front()->GetDesc();
        //    desc.BindFlags = 0;
        //    desc.Usage = D3D12_USAGE_STAGING;
        //    desc.CPUAccessFlags = D3D12_CPU_ACCESS_READ;

        //    this->_staging_texture = nullptr;
        //    hr = this->device()->CreateTexture2D(&desc, nullptr,
        //        &this->_staging_texture);
        //    if (FAILED(hr)) {
        //        throw ATL::CAtlException(hr);
        //    }
        //}
        //assert(this->_staging_texture != nullptr);

        //this->_device_context->CopyResource(this->_staging_texture, tex);

        //hr = this->_device_context->Map(this->_staging_texture, 0,
        //    D3D12_MAP_READ, 0, &map);
        //if (FAILED(hr)) {
        //    throw ATL::CAtlException(hr);
        //}

        //try {
        //    trrojan::wic_save(trrojan::get_wic_factory(), map.pData, desc.Width,
        //        desc.Height, map.RowPitch, GUID_WICPixelFormat32bppBGRA, path,
        //        GUID_NULL);
        //    this->_device_context->Unmap(this->_staging_texture, 0);
        //} catch (...) {
        //    this->_device_context->Unmap(this->_staging_texture, 0);
        //    throw;
        //}

    } else {
        log::instance().write_line(log_level::warning, "Benchmarking render "
            "target could not be saved, because it was not initialised.");
    }
}


/*
 * trrojan::d3d12::render_target_base::to_uav
 */
//void trrojan::d3d12::render_target_base::to_uav(
//        const D3D12_CPU_DESCRIPTOR_HANDLE dst,
//        ID3D12GraphicsCommandList *cmd_list) {
//    transition_subresource(cmd_list,
//        this->current_buffer(), 0,
//        D3D12_RESOURCE_STATE_RENDER_TARGET,
//        D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
//    this->device()->CreateUnorderedAccessView(this->current_buffer(), nullptr,
//        nullptr, dst);
//}


/*
 * trrojan::d3d12::render_target_base::use_reversed_depth_buffer
 */
void trrojan::d3d12::render_target_base::use_reversed_depth_buffer(
        const bool isEnabled) {
    throw "TODO";
    //this->_dss = nullptr;

    //if (isEnabled) {
    //    D3D12_DEPTH_STENCIL_DESC desc;
    //    ::ZeroMemory(&desc, sizeof(desc));

    //    desc.DepthEnable = TRUE;
    //    desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    //    desc.DepthFunc = D3D12_COMPARISON_GREATER;
    //    desc.StencilEnable = FALSE;

    //    auto hr = this->_device->CreateDepthStencilState(&desc, &this->_dss);
    //    if (FAILED(hr)) {
    //        throw ATL::CAtlException(hr);
    //    }

    //    this->_depth_clear = 0.0f;


    //} else {
    //    this->_depth_clear = 1.0f;
    //}

}


/*
 * trrojan::d3d12::render_target_base::wait_for_gpu
 */
void trrojan::d3d12::render_target_base::wait_for_gpu(void) {
    assert(this->_command_queue != nullptr);
    auto& fenceValue = this->_fence_values[this->_buffer_index];

    // Make the fence signal in the command queue with its current value.
    assert(this->_fence != nullptr);
    {
        auto hr = this->_command_queue->Signal(this->_fence, fenceValue);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Signal the event if the fence singalled with its current value.
    assert(this->_fence_event != NULL);
    {
        auto hr = this->_fence->SetEventOnCompletion(fenceValue,
            this->_fence_event);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Wait for the event to signal.
    switch (::WaitForSingleObjectEx(this->_fence_event, INFINITE, FALSE)) {
        case WAIT_FAILED:
            throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));

        default:
            break;
    }

    // Increase the fence for the next call ('fenceValue' is a ref!).
    ++fenceValue;
}


/*
 * trrojan::d3d12::render_target_base::render_target_base
 */
trrojan::d3d12::render_target_base::render_target_base(
        const trrojan::device& device, const UINT pipelineDepth)
    : _depth_clear(1.0f), _buffer_index(0), _buffers(pipelineDepth),
        _fence_values(pipelineDepth, 0), _rtv_descriptor_size(0) {
    auto d = std::dynamic_pointer_cast<trrojan::d3d12::device>(device);
    if (d == nullptr) {
        throw std::invalid_argument("The device passed to the Direct3D 12 "
            "render target must be a Direct3D 12 device.");
    }
    if (this->_buffers.size() < 2) {
        throw std::invalid_argument("A render target must be at least "
            "double-buffered.");
    }

    this->_command_queue = d->command_queue();
    this->_device = d->d3d_device();
    this->_dxgi_factory = d->dxgi_factory();
    ::ZeroMemory(&this->_viewport, sizeof(this->_viewport));

    {
        auto hr = this->_device->CreateFence(
            this->_fence_values[this->_buffer_index],
            D3D12_FENCE_FLAG_NONE,
            ::IID_ID3D12Fence,
            reinterpret_cast<void **>(&this->_fence));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        ++this->_fence_values[this->_buffer_index];
    }

    this->_fence_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!this->_fence_event) {
        throw ATL::CAtlException(HRESULT_FROM_WIN32(GetLastError()));
    }
}


/*
 * trrojan::d3d12::render_target_base::create_descriptor_heap
 */
ATL::CComPtr<ID3D12DescriptorHeap>
trrojan::d3d12::render_target_base::create_descriptor_heap(
        const D3D12_DESCRIPTOR_HEAP_TYPE type, const UINT cnt) {
    assert(this->_device != nullptr);
    ATL::CComPtr<ID3D12DescriptorHeap> retval;

    D3D12_DESCRIPTOR_HEAP_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.NumDescriptors = cnt;
    desc.Type = type;

    auto hr = this->_device->CreateDescriptorHeap(&desc,
        ::IID_ID3D12DescriptorHeap,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::render_target_base::create_dsv_heap
 */
void trrojan::d3d12::render_target_base::create_dsv_heap(void) {
    this->_dsv_heap = this->create_descriptor_heap(
        D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
}


/*
 * trrojan::d3d12::render_target_base::create_rtv_heap
 */
void trrojan::d3d12::render_target_base::create_rtv_heap(void) {
    this->_rtv_heap = this->create_descriptor_heap(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV, this->pipeline_depth());
    this->_rtv_descriptor_size = this->_device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}


/*
 * trrojan::d3d12::render_target_base::create_swap_chain
 */
ATL::CComPtr<IDXGISwapChain3>
trrojan::d3d12::render_target_base::create_swap_chain(HWND hWnd) {
    assert(this->_command_queue != nullptr);
    assert(this->_dxgi_factory != nullptr);
    assert(this->pipeline_depth() >= 1);
    ATL::CComPtr<IDXGISwapChain3> retval;
    UINT height = 1;
    ATL::CComPtr<IDXGISwapChain1> swapChain;
    UINT width = 1;

    {
        RECT clientRect;
        if (::GetClientRect(hWnd, &clientRect)) {
            height = std::abs(clientRect.bottom - clientRect.top);
            width = std::abs(clientRect.right - clientRect.left);
        }
    }

    {
        DXGI_SWAP_CHAIN_DESC1 desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferCount = this->pipeline_depth();
        desc.Width = width;
        desc.Height = height;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.SampleDesc.Count = 1;

        auto hr = this->_dxgi_factory->CreateSwapChainForHwnd(
            this->_command_queue, hWnd, &desc, nullptr, nullptr, &swapChain);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->_dxgi_factory->MakeWindowAssociation(hWnd,
            DXGI_MWA_NO_ALT_ENTER);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = swapChain.QueryInterface(&retval);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    return retval;
}


/*
 * trrojan::d3d12::render_target_base::current_buffer
 */
ATL::CComPtr<ID3D12Resource> trrojan::d3d12::render_target_base::current_buffer(
        void) {
    return this->_buffers[this->_buffer_index];
}


/*
 * trrojan::d3d12::render_target_base::current_rtv_handle
 */
D3D12_CPU_DESCRIPTOR_HANDLE
trrojan::d3d12::render_target_base::current_rtv_handle(void) {
    auto retval = this->_rtv_heap->GetCPUDescriptorHandleForHeapStart();
    retval.ptr += static_cast<SIZE_T>(this->_buffer_index)
        * this->_rtv_descriptor_size;
    return retval;
}


/*
 * trrojan::d3d12::render_target_base::reset_buffers
 */
void trrojan::d3d12::render_target_base::reset_buffers(void) {
    this->wait_for_gpu();

    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        this->_buffers[i] = nullptr;
        this->_fence_values[i] = this->_fence_values[this->_buffer_index];
    }
}


/*
 * trrojan::d3d12::render_target_base::set_buffers
 */
void trrojan::d3d12::render_target_base::set_buffers(
        const std::vector<ATL::CComPtr<ID3D12Resource>> &buffers,
        const UINT buffer_index) {
    if (buffers.size() < this->_buffers.size()) {
        throw std::invalid_argument("A buffer must be provided for each stage "
            "of the pipeline. You have provided less buffers than there are "
            "render target views.");
    }
    if (buffer_index >= this->pipeline_depth()) {
        throw std::invalid_argument("The current buffer index cannot be "
            "outside the range of valid buffers.");
    }

    // Retain the buffers.
    std::copy_n(buffers.begin(), this->_buffers.size(),
        this->_buffers.begin());

    // Reset the current buffer to the user-defined value. This allows swap
    // chain-based subclasses to force the current index of the swap chain.
    this->_buffer_index = buffer_index;

    // Lazily create the RTV heap.
    if (this->_rtv_heap == nullptr) {
        this->create_rtv_heap();
    }

    // Create the RTVs from the buffers.
    {
        auto handle = this->_rtv_heap->GetCPUDescriptorHandleForHeapStart();
        for (auto& b : this->_buffers) {
            this->_device->CreateRenderTargetView(b, nullptr, handle);
            handle.ptr += this->_rtv_descriptor_size;
        }
    }

    // Lazily create the DSV heap.
    if (this->_dsv_heap == nullptr) {
        this->create_dsv_heap();
    }

    // Create the depth buffer and the DSV.
    {
        ATL::CComPtr<ID3D12Resource> depthBuffer;

        {
            D3D12_CLEAR_VALUE clearValue;
            ::ZeroMemory(&clearValue, sizeof(clearValue));
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil.Depth = 1.0f;   // TODO: inverse depth?
            clearValue.DepthStencil.Stencil = 0;

            auto desc = buffers.front()->GetDesc();
            desc.Format = DXGI_FORMAT_D32_FLOAT;
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

            D3D12_HEAP_PROPERTIES props;
            ::ZeroMemory(&props, sizeof(props));
            props.Type = D3D12_HEAP_TYPE_DEFAULT;
            props.CreationNodeMask = 1;
            props.VisibleNodeMask = 1;

            auto hr = this->_device->CreateCommittedResource(&props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                ::IID_ID3D12Resource,
                reinterpret_cast<void **>(&depthBuffer));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            D3D12_DEPTH_STENCIL_VIEW_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.Format = DXGI_FORMAT_D32_FLOAT;
            desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

            this->_device->CreateDepthStencilView(depthBuffer, &desc,
                this->_dsv_heap->GetCPUDescriptorHandleForHeapStart());
        }
    }

    // Fill the viewport dimensions.
    {
        auto desc = buffers.front()->GetDesc();
        this->_viewport.Height = static_cast<float>(desc.Height);
        this->_viewport.MaxDepth = 1.0f;
        this->_viewport.MinDepth = 0.0f;
        this->_viewport.TopLeftX = 0.0f;
        this->_viewport.TopLeftY = 0.0f;
        this->_viewport.Width = static_cast<float>(desc.Width);
    }
}


/*
 * trrojan::d3d12::render_target_base::wait_for_frame
 */
void trrojan::d3d12::render_target_base::wait_for_frame(
        const UINT nextFrame) {
    assert(nextFrame < this->_fence_values.size());
    assert(nextFrame != this->_buffer_index);
    const auto currentFenceValue = this->_fence_values[this->_buffer_index];

    // Schedule a signal in the queue.
    assert(this->_fence != nullptr);
    {
        auto hr = this->_command_queue->Signal(this->_fence, currentFenceValue);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Move to next frame.
    this->_buffer_index = nextFrame;
    auto& fenceValue = this->_fence_values[_buffer_index];

    // If the next frame is not yet ready, wait for it.
    if (this->_fence->GetCompletedValue() < fenceValue) {
        assert(this->_fence_event != NULL);
        auto hr = this->_fence->SetEventOnCompletion(fenceValue,
            this->_fence_event);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        switch (::WaitForSingleObjectEx(this->_fence_event, INFINITE, FALSE)) {
            case WAIT_FAILED:
                throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));

            default:
                break;
        }
    }

    // Set the value for the next frame ('fenceValue' is ref!).
    fenceValue = currentFenceValue + 1;
}
