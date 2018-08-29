/// <copyright file="device.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/device.h"

#include <cassert>
#include <codecvt>
#include <locale>

#include "trrojan/log.h"


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
                "adapter for device %p (error 0x%x).\n", (void *) device.p, hr);
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
                "factory for adapter %p (error 0x%x).\n", (void *)adapter.p, hr);
            retval = nullptr;
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::device::device
 */
trrojan::d3d11::device::device(ATL::CComPtr<ID3D11Device> d3dDevice)
        : d3dDevice(d3dDevice) {
    assert(this->d3dDevice != nullptr);
    DXGI_ADAPTER_DESC desc;
    auto adapter = device::get_dxgi_adapter(this->d3dDevice);
    HRESULT hr = (adapter != nullptr) ? S_OK : E_POINTER;

    if (SUCCEEDED(hr)) {
        hr = adapter->GetDesc(&desc);
    }

    if (SUCCEEDED(hr)) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        this->_name = conv.to_bytes(desc.Description);
        this->_unique_id = desc.DeviceId;
    }

    if (SUCCEEDED(hr)) {
        this->d3dDevice->GetImmediateContext(&this->d3dContext);
    }
}


/*
 * trrojan::d3d11::device::~device
 */
trrojan::d3d11::device::~device(void) { }
