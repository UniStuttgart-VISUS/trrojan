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
 * trrojan::d3d12::device::device
 */
trrojan::d3d12::device::device(const ATL::CComPtr<ID3D12Device>& d3dDevice,
        const ATL::CComPtr<IDXGIFactory4>& dxgiFactory)
        : _d3d_device(d3dDevice), _dxgi_factory(dxgiFactory) {
    assert(this->_d3d_device != nullptr);
    assert(this->_dxgi_factory != nullptr);
    DXGI_ADAPTER_DESC desc;
    auto adapter = this->dxgi_adapter();
    HRESULT hr = (adapter != nullptr) ? S_OK : E_POINTER;

    if (SUCCEEDED(hr)) {
        hr = adapter->GetDesc(&desc);
    }

    if (SUCCEEDED(hr)) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        this->_name = conv.to_bytes(desc.Description);
        this->_unique_id = desc.DeviceId;
    }

    //if (SUCCEEDED(hr)) {
    //    this->d3dDevice->GetImmediateContext(&this->d3dContext);
    //}
}


/*
 * trrojan::d3d12::device::~device
 */
trrojan::d3d12::device::~device(void) { }


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
