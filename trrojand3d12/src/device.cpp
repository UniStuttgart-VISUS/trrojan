// <copyright file="device.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/device.h"

#include <cassert>
#include <codecvt>
#include <locale>

#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"



/*
 * trrojan::d3d12::device::create_command_queue
 */
ATL::CComPtr<ID3D12CommandQueue> trrojan::d3d12::device::create_command_queue(
        ID3D12Device *device, const D3D12_COMMAND_LIST_TYPE type) {
    if (device == nullptr) {
        throw std::invalid_argument("A valid device must be provided to create "
            "a command queue.");
    }

    D3D12_COMMAND_QUEUE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Type = type;

    ATL::CComPtr<ID3D12CommandQueue> retval;
    auto hr = device->CreateCommandQueue(&desc, ::IID_ID3D12CommandQueue,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d12::device::device
 */
trrojan::d3d12::device::device(const ATL::CComPtr<ID3D12Device>& d3dDevice,
        const ATL::CComPtr<IDXGIFactory4>& dxgiFactory)
    : _d3d_device(d3dDevice),
        _dxgi_factory(dxgiFactory),
        _next_fence(0) {
    this->_command_queue = this->make_cmd_queue();
    this->_fence = this->make_fence();
    this->set_desc();
    assert(this->_command_queue != nullptr);
    assert(this->_d3d_device != nullptr);
    assert(this->_fence != nullptr);
}
#endif /* !defined(TRROJAN_FOR_UWP) */


/*
 * trrojan::d3d12::device::close_and_execute_command_list
 */
void trrojan::d3d12::device::close_and_execute_command_list(
        ID3D12GraphicsCommandList *cmd_list) {
    close_command_list(cmd_list);
    this->execute_command_list(cmd_list);
}


/*
 * trrojan::d3d12::device::dxgi_adapter
 */
ATL::CComPtr<IDXGIAdapter> trrojan::d3d12::device::dxgi_adapter(void) {
    assert(this->d3d_device() != nullptr);
    assert(this->_dxgi_factory != nullptr);
    ATL::CComPtr<IDXGIAdapter> retval;

    auto hr = this->_dxgi_factory->EnumAdapterByLuid(
        this->d3d_device()->GetAdapterLuid(),
        IID_IDXGIAdapter,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::device::execute_command_list
 */
void trrojan::d3d12::device::execute_command_list(ID3D12CommandList *cmd_list) {
    assert(cmd_list != nullptr);
    this->command_queue()->ExecuteCommandLists(1, &cmd_list);
}


/*
 * trrojan::d3d12::device::set_stable_power_state
 */
void trrojan::d3d12::device::set_stable_power_state(const bool enabled) {
    assert(this->d3d_device() != nullptr);
    auto hr = this->d3d_device()->SetStablePowerState(enabled);
    if (FAILED(hr)) {
        throw CAtlException(hr);
    }
}


/*
 * trrojan::d3d12::device::wait_for_gpu
 */
void trrojan::d3d12::device::wait_for_gpu(void) {
    auto value = this->_next_fence++;

    // Make the fence signal in the command queue with its current value.
    {
        auto hr = this->command_queue()->Signal(this->fence(), value);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Signal the event if the fence signalled with its current value.
    auto evt = create_event(false, false);
    {
        auto hr = this->fence()->SetEventOnCompletion(value, evt);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Wait for the event to signal.
    wait_for_event(evt);
}


/*
 * trrojan::d3d12::device::make_cmd_queue
 */
ATL::CComPtr<ID3D12CommandQueue> trrojan::d3d12::device::make_cmd_queue(void) {
    auto retval = create_command_queue(this->d3d_device());
    set_debug_object_name(retval, "Device command queue \"{0}\"", this->name());
    return retval;
}


/*
 * trrojan::d3d12::device::make_fence
 */
ATL::CComPtr<ID3D12Fence> trrojan::d3d12::device::make_fence(void) {
    ATL::CComPtr<ID3D12Fence> retval;

    auto hr = this->d3d_device()->CreateFence(this->_next_fence++,
        D3D12_FENCE_FLAG_NONE, ::IID_ID3D12Fence,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    set_debug_object_name(retval, "Device fence \"{0}\"", this->name());
    return retval;
}


/*
 * trrojan::d3d12::device::set_desc
 */
void trrojan::d3d12::device::set_desc(void) {
    assert(this->d3d_device() != nullptr);
    if (this->_dxgi_factory == nullptr) {
        throw std::invalid_argument("The DXGI factory passed to a TRRojan "
            "device wrapper must not be nullptr.");
    }

    DXGI_ADAPTER_DESC desc;
    auto hr = this->dxgi_adapter()->GetDesc(&desc);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    this->_name = conv.to_bytes(desc.Description);
    this->_unique_id = desc.DeviceId;
}
