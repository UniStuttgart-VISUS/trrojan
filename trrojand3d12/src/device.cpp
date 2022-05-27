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



/*
 * trrojan::d3d12::device::create_command_allocator
 */
ATL::CComPtr<ID3D12CommandAllocator>
trrojan::d3d12::device::create_command_allocator(ID3D12Device *device,
        const D3D12_COMMAND_LIST_TYPE type) {
    if (device == nullptr) {
        throw std::invalid_argument("A valid device must be provided to create "
            "a command allocator.");
    }

    ATL::CComPtr<ID3D12CommandAllocator> retval;
    auto hr = device->CreateCommandAllocator(type, ::IID_ID3D12CommandQueue,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


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


/*
 * trrojan::d3d12::device::device
 */
trrojan::d3d12::device::device(const ATL::CComPtr<ID3D12Device>& d3dDevice,
        const ATL::CComPtr<IDXGIFactory4>& dxgiFactory)
    : _command_queue(create_command_queue(d3dDevice)),
        _compute_command_allocator(create_command_allocator(d3dDevice,
            D3D12_COMMAND_LIST_TYPE_COMPUTE)),
        _copy_command_allocator(create_command_allocator(d3dDevice,
            D3D12_COMMAND_LIST_TYPE_COPY)),
        _d3d_device(d3dDevice),
        _direct_command_allocator(create_command_allocator(d3dDevice,
            D3D12_COMMAND_LIST_TYPE_DIRECT)),
        _dxgi_factory(dxgiFactory) {
    assert(this->_command_queue != nullptr);
    assert(this->_compute_command_allocator != nullptr);
    assert(this->_copy_command_allocator != nullptr);
    assert(this->_d3d_device != nullptr);
    assert(this->_direct_command_allocator != nullptr);
    DXGI_ADAPTER_DESC desc;

    if (this->_dxgi_factory == nullptr) {
        throw std::invalid_argument("The DXGI factory passed to a TRRojan "
            "device wrapper must not be nullptr.");
    }

    {
        auto hr = this->dxgi_adapter()->GetDesc(&desc);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        this->_name = conv.to_bytes(desc.Description);
        this->_unique_id = desc.DeviceId;
    }
}


/*
 * trrojan::d3d12::device::~device
 */
trrojan::d3d12::device::~device(void) { }


/*
 * trrojan::d3d12::device::create_command_list
 */
ATL::CComPtr<ID3D12CommandList> trrojan::d3d12::device::create_command_list(
        const D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState *initial_state) {
    assert(this->_compute_command_allocator != nullptr);
    assert(this->_copy_command_allocator != nullptr);
    assert(this->_d3d_device != nullptr);
    assert(this->_direct_command_allocator != nullptr);
    auto hr = S_OK;
    ATL::CComPtr<ID3D12CommandList> retval;

    switch (type) {
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            hr = this->_d3d_device->CreateCommandList(0, type,
                this->_compute_command_allocator, initial_state,
                ::IID_ID3D12CommandList, reinterpret_cast<void **>(&retval));
            break;

        case D3D12_COMMAND_LIST_TYPE_COPY:
            hr = this->_d3d_device->CreateCommandList(0, type,
                this->_copy_command_allocator, initial_state,
                ::IID_ID3D12CommandList, reinterpret_cast<void **>(&retval));
            break;

        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            hr = this->_d3d_device->CreateCommandList(0, type,
                this->_direct_command_allocator, initial_state,
                ::IID_ID3D12CommandList, reinterpret_cast<void **>(&retval));
            break;

        default:
            throw std::invalid_argument("The specified type of command list is "
                "not supported by this convenience method.");
    }

    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::device::create_graphics_command_list
 */
ATL::CComPtr<ID3D12GraphicsCommandList>
trrojan::d3d12::device::create_graphics_command_list(
        const D3D12_COMMAND_LIST_TYPE type,
        ID3D12PipelineState *initial_state) {
    auto cmd_list = this->create_command_list(type, initial_state);
    ATL::CComPtr<ID3D12GraphicsCommandList> retval;
    auto hr = cmd_list.QueryInterface(&retval);

    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::device::dxgi_adapter
 */
ATL::CComPtr<IDXGIAdapter> trrojan::d3d12::device::dxgi_adapter(void) {
    assert(this->_d3d_device != nullptr);
    assert(this->_dxgi_factory != nullptr);
    ATL::CComPtr<IDXGIAdapter> retval;

    auto hr = this->_dxgi_factory->EnumAdapterByLuid(
        this->_d3d_device->GetAdapterLuid(),
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
    assert(this->_d3d_device != nullptr);
    auto hr = this->_d3d_device->SetStablePowerState(enabled);
    if (FAILED(hr)) {
        throw CAtlException(hr);
    }
}
