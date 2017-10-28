/// <copyright file="device.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/device.h"

#include <cassert>
#include <codecvt>
#include <locale>


/*
 * trrojan::d3d11::device::device
 */
trrojan::d3d11::device::device(ATL::CComPtr<ID3D11Device> d3dDevice) 
        : d3dDevice(d3dDevice) {
    assert(this->d3dDevice != nullptr);
    ATL::CComPtr<IDXGIAdapter> adapter;
    DXGI_ADAPTER_DESC desc;
    ATL::CComPtr<IDXGIDevice> dxgi;
    HRESULT hr = (this->d3dDevice != nullptr) ? S_OK : E_POINTER;

    if (SUCCEEDED(hr)) {
        hr = this->d3dDevice->QueryInterface(&dxgi);
    }

    if (SUCCEEDED(hr)) {
        hr = dxgi->GetAdapter(&adapter);
    }

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
