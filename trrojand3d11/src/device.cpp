// <copyright file="device.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/device.h"

#include <cassert>
#include <codecvt>
#include <locale>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::device::get_dxgi_adapter
 */
ATL::CComPtr<ID3D11DeviceContext> trrojan::d3d11::device::get_context(
        ATL::CComPtr<ID3D11Device> device) {
    assert(device != nullptr);
    ATL::CComPtr<ID3D11DeviceContext> retval;
    device->GetImmediateContext(&retval);
    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_adapter
 */
ATL::CComPtr<IDXGIAdapter> trrojan::d3d11::device::get_dxgi_adapter(
        ATL::CComPtr<ID3D11Device> device) {
    ATL::CComPtr<IDXGIAdapter> retval;

    auto dxgiDevice = device::get_dxgi_device(device);
    if (dxgiDevice != nullptr) {
        auto hr = dxgiDevice->GetAdapter(&retval);
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "adapter for device {} (error {:x}).\n",
                static_cast<void *>(device.p), hr);
            retval = nullptr;
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_device
 */
ATL::CComPtr<IDXGIDevice> trrojan::d3d11::device::get_dxgi_device(
        ATL::CComPtr<ID3D11Device> device) {
    ATL::CComPtr<IDXGIDevice> retval;

    if (trrojan::d3d11::device::get_dxgi_device != nullptr) {
        auto hr = device->QueryInterface(&retval);
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "device for Direct3D device %p (error 0x%x).\n", (void *)device.p,
                hr);
            retval = nullptr;
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::device::get_dxgi_factory
 */
ATL::CComPtr<IDXGIFactory> trrojan::d3d11::device::get_dxgi_factory(
        ATL::CComPtr<ID3D11Device> device) {
    ATL::CComPtr<IDXGIFactory> retval;

    auto adapter = device::get_dxgi_adapter(device);
    if (adapter != nullptr) {
        auto hr = adapter->GetParent(IID_IDXGIFactory,
            reinterpret_cast<void **>(&retval));
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to retrieve DXGI "
                "factory for adapter {} (error {:x}).\n",
                static_cast<void *>(adapter.p), hr);
            retval = nullptr;
        }
    }

    return retval;
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d11::device::device
 */
trrojan::d3d11::device::device(ATL::CComPtr<ID3D11Device> d3d_device)
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
        throw ATL::CAtlException(E_POINTER);
    }

    DXGI_ADAPTER_DESC desc;
    auto hr = adapter->GetDesc(&desc);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    this->_name = conv.to_bytes(desc.Description);
    this->_unique_id = desc.DeviceId;
}
