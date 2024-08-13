// <copyright file="environment.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/environment.h"

#include <cassert>
#include <cinttypes>
#include <iterator>
#include <memory>
#include <set>

#include <Windows.h>
#include <atlbase.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#include "trrojan/cmd_line.h"
#include "trrojan/log.h"
#include "trrojan/text.h"

#include "trrojan/d3d12/utilities.h"


/*
 * trrojan::d3d12::environment::~environment
 */
trrojan::d3d12::environment::~environment(void) { }


/*
 * trrojan::d3d12::environment::get_devices
 */
size_t trrojan::d3d12::environment::get_devices(device_list& dst) {
    for (auto d : this->_devices) {
        auto dd = std::dynamic_pointer_cast<trrojan::device_base>(d);
        assert(dd != nullptr);
        dst.push_back(std::move(dd));
    }
    return this->_devices.size();
}


/*
 * trrojan::d3d12::environment::on_activate
 */
void trrojan::d3d12::environment::on_activate(void) { }


/*
 * trrojan::d3d12::environment::on_deactivate
 */
void trrojan::d3d12::environment::on_deactivate(void) {
#if defined(TRROJAN_FOR_UWP)
    // See ctor for why we need to do that on UWP.
    log::instance().write_line(log_level::information,
        "Cleaning up Direct3D devices of \"{}\" as part of deactiviating "
        "the environment.", this->name());

    for (auto &d : this->_devices) {
        d.reset();
    }
#endif /* defined(TRROJAN_FOR_UWP) */
}


/*
 * trrojan::d3d12::environment::on_finalise
 */
void trrojan::d3d12::environment::on_finalise(void) {
    this->_devices.clear();
#if !defined(TRROJAN_FOR_UWP)
    ::CoUninitialize();
#endif /* !defined(TRROJAN_FOR_UWP) */
}


/*
 * trrojan::d3d12::environment::on_initialise
 */
void trrojan::d3d12::environment::on_initialise(const cmd_line& cmdLine) {
    USES_CONVERSION;
    ATL::CComPtr<IDXGIFactory4> factory;
    UINT factoryFlags = 0;
    const auto isBasicRender = contains_switch("--with-basic-render-driver",
        cmdLine.begin(), cmdLine.end());
    const auto isUniqueDevice = contains_switch("--unique-devices",
        cmdLine.begin(), cmdLine.end());
    std::set<std::pair<UINT, UINT>> pciIds;

#if !defined(TRROJAN_FOR_UWP)
    // Initialise COM (for WIC).
    {
        auto hr = ::CoInitialize(nullptr);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
#endif /* !defined(TRROJAN_FOR_UWP) */

    // Enable the debug layer in debug builds, which requires the debugging
    // layer being installed like for D3D11. Enabling the debug layer after
    // device creation will invalidate the active device.
    // Note: For unknown reason, enabling the debug layer on Xbox does not work,
    // so we have an additional switch which allows us to force the debug layer
    // turned off.
#if !defined(TRROJAN_FORCE_NO_D3D_DEBUG) && (defined(DEBUG) || defined(_DEBUG))
    {
        ATL::CComPtr<ID3D12Debug> debug;
        ATL::CComPtr<ID3D12Debug1> debug1;

        {
            auto hr = ::D3D12GetDebugInterface(::IID_ID3D12Debug,
                reinterpret_cast<void **>(&debug));
            if (SUCCEEDED(hr)) {
                debug->EnableDebugLayer();

                // If the debug layer is installed, enable debugging in DXGI
                // as well.
                factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }

        // If supported, enable GPU-based validation.
        {
            auto hr = debug->QueryInterface(IID_PPV_ARGS(&debug1));
            if (SUCCEEDED(hr)) {
                debug1->SetEnableGPUBasedValidation(true);
            }
        }
    }

    //{
    //    ATL::CComPtr<ID3D12InfoQueue> iq;

    //    hr = ::DXGIGetDebugInterface1(0, IID_PPV_ARGS(&iq.p));
    //    if (SUCCEEDED(hr)) {
    //        auto flags = DXGI_CREATE_FACTORY_DEBUG;

    //        iq->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
    //        iq->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

    //        DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
    //            80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
    //        };

    //        DXGI_INFO_QUEUE_FILTER filter = { };
    //        filter.DenyList.NumIDs = _countof(hide);
    //        filter.DenyList.pIDList = hide;

    //        iq->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
    //    }
    //}
#endif /* !defined(TRROJAN_FORCE_NO_D3D_DEBUG) && (defined(DEBUG) ... */

    // Create DXGI factory.
    {
        auto hr = ::CreateDXGIFactory2(factoryFlags, IID_IDXGIFactory4,
            reinterpret_cast<void **>(&factory));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = S_OK;

        for (UINT a = 0; SUCCEEDED(hr) || (hr != DXGI_ERROR_NOT_FOUND); ++a) {
            ATL::CComPtr<IDXGIAdapter> adapter;
            DXGI_ADAPTER_DESC desc;
            ATL::CComPtr<ID3D12Device> device;

            hr = factory->EnumAdapters(a, &adapter);
            if (SUCCEEDED(hr)) {
                hr = adapter->GetDesc(&desc);
            }

            if (SUCCEEDED(hr)) {
                if ((desc.VendorId == 0x1414) && (desc.DeviceId == 0x8c)
                        && !isBasicRender) {
                    // Skip Microsoft's software emulation (cf.
                    // https://msdn.microsoft.com/en-us/library/windows/desktop/bb205075(v=vs.85).aspx)
                    log::instance().write_line(log_level::information,
                        "Excluding \"{}\" from list of device eligible for "
                        "benchmarking because --with-basic-render-driver was "
                        "not specified.", W2A(desc.Description));
                    continue;
                }

                if (isUniqueDevice) {
                    auto pciId = std::make_pair(desc.VendorId, desc.DeviceId);
                    if (std::find(pciIds.begin(), pciIds.end(), pciId)
                            != pciIds.end()) {
                        log::instance().write_line(log_level::information,
                            "Excluding \"{}\" from list of device eligible for "
                            "benchmarking because another device with the same "
                            "PCI ID was already added.", W2A(desc.Description));
                        continue;
                    } else {
                        pciIds.emplace(std::move(pciId));
                    }
                }
            }

#if !defined(TRROJAN_FOR_UWP)
            if (SUCCEEDED(hr)) {
                hr = ::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1,
                    ::IID_ID3D12Device, reinterpret_cast<void **>(&device));
            }
#endif /* !defined(TRROJAN_FOR_UWP) */

            if (SUCCEEDED(hr)) {
#if defined(TRROJAN_FOR_UWP)
                // On UWP, we must generate the D3D devices lazily as we cannot have
                // a D3D11 device and a D3D12 device open at the same time on Xbox.
                // For this reason, the environment must reset all devices it has
                // created during benchmarks once it is deactivated.
                this->_devices.push_back(std::make_shared<d3d12::device>(
                        factory, [adapter](void) {
                    ATL::CComPtr<ID3D12Device> retval;
                    auto hr = ::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                        ::IID_ID3D12Device, reinterpret_cast<void **>(&retval));
                    if (FAILED(hr)) {
                        throw ATL::CAtlException(hr);
                    }
                    return retval;
                }));
#else /* !defined(TRROJAN_FOR_UWP) */
                this->_devices.push_back(std::make_shared<d3d12::device>(device,
                    factory));
#endif /* !defined(TRROJAN_FOR_UWP) */
            }
        }

        if (FAILED(hr) && (hr != DXGI_ERROR_NOT_FOUND)) {
            throw ATL::CAtlException(hr);
        }
    }
}
