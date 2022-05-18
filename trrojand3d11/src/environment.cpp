/// <copyright file="environment.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/environment.h"

#include <cassert>
#include <cinttypes>
#include <iterator>
#include <memory>
#include <set>

#include <Windows.h>
#include <atlbase.h>
#include <dxgi.h>

#include "trrojan/cmd_line.h"
#include "trrojan/log.h"
#include "trrojan/text.h"

#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::environment::~environment
 */
trrojan::d3d11::environment::~environment(void) { }


/*
 * trrojan::d3d11::environment::get_devices
 */
size_t trrojan::d3d11::environment::get_devices(device_list& dst) {
    for (auto d : this->_devices) {
        auto dd = std::dynamic_pointer_cast<trrojan::device_base>(d);
        assert(dd != nullptr);
        dst.push_back(std::move(dd));
    }
    return this->_devices.size();
}


/*
 * trrojan::d3d11::environment::on_activate
 */
void trrojan::d3d11::environment::on_activate(void) { }


/*
 * trrojan::d3d11::environment::on_deactivate
 */
void trrojan::d3d11::environment::on_deactivate(void) { }


/*
 * trrojan::d3d11::environment::on_finalise
 */
void trrojan::d3d11::environment::on_finalise(void) {
    this->_devices.clear();
#ifndef _UWP
    ::CoUninitialize();
#endif
}


/*
 * trrojan::d3d11::environment::on_initialise
 */
void trrojan::d3d11::environment::on_initialise(const cmd_line& cmdLine) {
    USES_CONVERSION;
    DWORD deviceFlags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
    ATL::CComPtr<IDXGIFactory> factory;
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = S_OK;
    const auto isBasicRender = contains_switch("--with-basic-render-driver",
        cmdLine.begin(), cmdLine.end());
    const auto isUniqueDevice = contains_switch("--unique-devices",
        cmdLine.begin(), cmdLine.end());
    std::set<std::pair<UINT, UINT>> pciIds;

    /* Initialise COM (for WIC). */
#ifndef _UWP

    hr = ::CoInitialize(nullptr);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
#endif

    /* Create DXGI factory. */
    hr = ::CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void **>(
        &factory));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

#if (defined(DEBUG) || defined(_DEBUG))
    if (supports_debug_layer()) {
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

    for (UINT a = 0; SUCCEEDED(hr) || (hr != DXGI_ERROR_NOT_FOUND); ++a) {
        ATL::CComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC desc;
        ATL::CComPtr<ID3D11Device> device;

        hr = factory->EnumAdapters(a, &adapter);
        if (SUCCEEDED(hr)) {
            hr = adapter->GetDesc(&desc);
        }

        if (SUCCEEDED(hr)) {
            if ((desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c)
                    && !isBasicRender) {
                // Skip Microsoft's software emulation (cf.
                // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx)
                log::instance().write_line(log_level::information, "Excluding "
                    "\"%s\" from list of device eligible for benchmarking "
                    "because --with-basic-render-driver was not specified.",
                    W2A(desc.Description));
                continue;
            }

            if (isUniqueDevice) {
                auto pciId = std::make_pair(desc.VendorId, desc.DeviceId);
                if (std::find(pciIds.begin(), pciIds.end(), pciId)
                        != pciIds.end()) {
                    log::instance().write_line(log_level::information,
                        "Excluding \"%s\" from list of device eligible for "
                        "benchmarking because another device with the same PCI "
                        "IDs was already added.", W2A(desc.Description));
                    continue;
                } else {
                    pciIds.emplace(std::move(pciId));
                }
            }
        }

        if (SUCCEEDED(hr)) {
            hr = ::D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
                deviceFlags, NULL, 0, D3D11_SDK_VERSION, &device,
                &featureLevel, nullptr);//&immediateContext);
        }

        if (SUCCEEDED(hr)) {
            this->_devices.push_back(std::make_shared<d3d11::device>(device));
        }
    }

    if (FAILED(hr) && (hr != DXGI_ERROR_NOT_FOUND)) {
        throw ATL::CAtlException(hr);
    }
}
