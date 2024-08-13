// <copyright file="device.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/device.h"

#include <cassert>
#include <codecvt>
#include <locale>

#include "trrojan/com_error_category.h"
#include "trrojan/log.h"


/*
 * trrojan::d3d11::device::get_dxgi_adapter
 */
winrt::com_ptr<ID3D11DeviceContext> trrojan::d3d11::device::get_context(
        winrt::com_ptr<ID3D11Device> device) {
    assert(device != nullptr);
    winrt::com_ptr<ID3D11DeviceContext> retval;
    device->GetImmediateContext(retval.put());
    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_adapter
 */
winrt::com_ptr<IDXGIAdapter> trrojan::d3d11::device::get_dxgi_adapter(
        winrt::com_ptr<ID3D11Device> device) {
    winrt::com_ptr<IDXGIAdapter> retval;

    auto dxgiDevice = device::get_dxgi_device(device);
    if (dxgiDevice != nullptr) {
        auto hr = dxgiDevice->GetAdapter(retval.put());
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "adapter for device {} (error {:x}).\n",
                static_cast<void *>(device.get()), hr);
            retval = nullptr;
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_device
 */
winrt::com_ptr<IDXGIDevice> trrojan::d3d11::device::get_dxgi_device(
        winrt::com_ptr<ID3D11Device> device) {
    winrt::com_ptr<IDXGIDevice> retval;

    if (trrojan::d3d11::device::get_dxgi_device != nullptr) {
        if (!device.try_as(retval)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "device for Direct3D device %p .\n",
                static_cast<void *>(device.get()));
            retval = nullptr;
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_factory
 */
winrt::com_ptr<IDXGIFactory> trrojan::d3d11::device::get_dxgi_factory(
        winrt::com_ptr<ID3D11Device> device) {
    winrt::com_ptr<IDXGIFactory> retval;

    auto adapter = device::get_dxgi_adapter(device);
    if (adapter != nullptr) {
        auto hr = adapter->GetParent(IID_IDXGIFactory, retval.put_void());
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "factory for adapter {} (error {:x}).\n",
                static_cast<void *>(adapter.get()), hr);
            retval = nullptr;
        }
    }

    return retval;
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d11::device::device
 */
trrojan::d3d11::device::device(winrt::com_ptr<ID3D11Device> d3d_device)
        : _d3d_context(get_context(d3d_device)), _d3d_device(d3d_device) {
    this->set_desc_from_device();
}
#endif /* !defined(TRROJAN_FOR_UWP) */


/*
 * trrojan::d3d11::device::set_desc_from_device
 */
void trrojan::d3d11::device::set_desc_from_device(void) {
    assert(this->d3d_device() != nullptr);
    auto adapter = device::get_dxgi_adapter(this->d3d_device());
    if (adapter == nullptr) {
        throw std::system_error(E_POINTER, com_category());
    }

    DXGI_ADAPTER_DESC desc;
    auto hr = adapter->GetDesc(&desc);
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    this->_name = conv.to_bytes(desc.Description);
    this->_unique_id = desc.DeviceId;
}
