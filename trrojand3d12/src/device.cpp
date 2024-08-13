// <copyright file="device.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/device.h"

#include <cassert>
#include <codecvt>
#include <locale>

#include "trrojan/com_error_category.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"


/*
 * trrojan::d3d12::device::create_command_queue
 */
winrt::com_ptr<ID3D12CommandQueue> trrojan::d3d12::device::create_command_queue(
        ID3D12Device *device, const D3D12_COMMAND_LIST_TYPE type) {
    if (device == nullptr) {
        throw std::invalid_argument("A valid device must be provided to create "
            "a command queue.");
    }

    D3D12_COMMAND_QUEUE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Type = type;

    winrt::com_ptr<ID3D12CommandQueue> retval;
    auto hr = device->CreateCommandQueue(&desc, ::IID_ID3D12CommandQueue,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d12::device::device
 */
trrojan::d3d12::device::device(const winrt::com_ptr<ID3D12Device>& d3dDevice,
        const winrt::com_ptr<IDXGIFactory4>& dxgiFactory)
    : _d3d_device(d3dDevice),
        _dxgi_factory(dxgiFactory),
        _next_fence(0) {
    this->_command_queue = this->make_cmd_queue();
    this->_fence = this->make_fence();
    this->set_desc_from_device();
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
winrt::com_ptr<IDXGIAdapter> trrojan::d3d12::device::dxgi_adapter(void) {
    assert(this->d3d_device() != nullptr);
    assert(this->_dxgi_factory != nullptr);
#if defined(TRROJAN_FOR_UWP)
    return get_adapter(this->_d3d_device.get().get(),
        this->_dxgi_factory.get());
#else /* defined(TRROJAN_FOR_UWP) */
    return get_adapter(this->_d3d_device.get(), this->_dxgi_factory.get());
#endif /* defined(TRROJAN_FOR_UWP) */
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
        throw std::system_error(hr, com_category());
    }
}


/*
 * trrojan::d3d12::device::wait_for_gpu
 */
void trrojan::d3d12::device::wait_for_gpu(void) {
    auto fence = this->fence();
    auto value = this->_next_fence++;

    // Signal the event if the fence signalled with its current value.
    auto evt = create_event(false, false);
    {
        log::instance().write_line(log_level::debug, "Set event on {0}.",
            value);
        auto hr = fence->SetEventOnCompletion(value, evt);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    // Make the fence signal in the command queue with its current value.
    {
        log::instance().write_line(log_level::debug, "Signal 0x{0:p} with {1}.",
            static_cast<void *>(fence.get()), value);
        auto hr = this->command_queue()->Signal(fence.get(), value);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

#if (defined(DEBUG) || defined(_DEBUG))
    log::instance().write_line(log_level::debug, "Current fence value is {0}.",
        fence->GetCompletedValue());
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

    // Wait for the event to signal.
    log::instance().write_line(log_level::debug, "Waiting for fence value {0}.",
        value);
    wait_for_event(evt);
}


/*
 * trrojan::d3d12::device::make_cmd_queue
 */
winrt::com_ptr<ID3D12CommandQueue> trrojan::d3d12::device::make_cmd_queue(void) {
    auto retval = create_command_queue(this->d3d_device());
    set_debug_object_name(retval, "Device command queue \"{0}\"", this->name());
    return retval;
}


/*
 * trrojan::d3d12::device::make_fence
 */
winrt::com_ptr<ID3D12Fence> trrojan::d3d12::device::make_fence(void) {
    winrt::com_ptr<ID3D12Fence> retval;

    auto hr = this->d3d_device()->CreateFence(this->_next_fence++,
        D3D12_FENCE_FLAG_NONE, ::IID_ID3D12Fence,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    set_debug_object_name(retval, "Device fence \"{0}\"", this->name());
    return retval;
}


/*
 * trrojan::d3d12::device::set_desc_from_device
 */
void trrojan::d3d12::device::set_desc_from_device(void) {
    assert(this->d3d_device() != nullptr);
    if (this->_dxgi_factory == nullptr) {
        throw std::invalid_argument("The DXGI factory passed to a TRRojan "
            "device wrapper must not be nullptr.");
    }

    DXGI_ADAPTER_DESC desc;
    auto hr = this->dxgi_adapter()->GetDesc(&desc);
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    this->_name = conv.to_bytes(desc.Description);
    this->_unique_id = desc.DeviceId;
}
