/// <copyright file="environment.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#include "trrojan/d3d11/environment.h"

#include <iterator>
#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <dxgi.h>


/*
 * trrojan::d3d11::environment::~environment
 */
trrojan::d3d11::environment::~environment(void) { }


/*
 * trrojan::d3d11::environment::get_devices
 */
size_t trrojan::d3d11::environment::get_devices(device_list& dst) {
    std::transform(this->devices.cbegin(), this->devices.cend(),
        std::back_inserter(dst), [](device::pointer d) {
        return std::dynamic_pointer_cast<trrojan::device_base>(d); });
    return this->devices.size();
}


/*
 * trrojan::d3d11::environment::on_activate
 */
void trrojan::d3d11::environment::on_activate(void) { }


/*
 * trrojan::d3d11::environment::on_deactivate
 */
void trrojan::d3d11::environment::on_deactivate(void)  noexcept { }


/*
 * trrojan::d3d11::environment::on_finalise
 */
void trrojan::d3d11::environment::on_finalise(void)  noexcept {
    this->devices.clear();
}


/*
 * trrojan::d3d11::environment::on_initialise
 */
void trrojan::d3d11::environment::on_initialise(const cmd_line& cmdLine) {
    DWORD deviceFlags = 0;
    ATL::CComPtr<IDXGIFactory> factory;
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = S_OK;

    /* Create DXGI factory. */
    hr = ::CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void **>(
        &factory));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    // Note: If the device creation fails on Windows 10, add the debug layer by
    // issuing
    //
    // Dism /online /add-capability /capabilityname:Tools.Graphics.DirectX~~~~0.0.1.0
    //
    // in an elevated command promt. The error message that D3D issues about
    // the SDK being not installed is wrong. See
    // http://stackoverflow.com/questions/32809169/use-d3d11-debug-layer-with-vs2013-on-windows-10
#if (defined(DEBUG) || defined(_DEBUG))
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

    for (UINT a = 0; SUCCEEDED(hr) || (hr != DXGI_ERROR_NOT_FOUND); ++a) {
        ATL::CComPtr<IDXGIAdapter> adapter;
        ATL::CComPtr<ID3D11Device> device;

        hr = factory->EnumAdapters(a, &adapter);
        if (SUCCEEDED(hr)) {
            hr = ::D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
                deviceFlags, NULL, 0, D3D11_SDK_VERSION, &device,
                &featureLevel, nullptr);//&immediateContext);
        }
        if (SUCCEEDED(hr)) {
            this->devices.push_back(std::make_shared<d3d11::device>(device));
        }
    }

    if (FAILED(hr) && (hr != DXGI_ERROR_NOT_FOUND)) {
        throw ATL::CAtlException(hr);
    }
}