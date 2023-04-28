/// <copyright file="power_state_scope.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#include "trrojan/power_state_scope.h"

#include "trrojan/log.h"


/*
 * trrojan::power_state_scope::power_state_scope
 */
trrojan::power_state_scope::power_state_scope(void) {
#if defined(TRROJAN_WITH_POWER_STATE_SCOPE)
    USES_CONVERSION;
#define LOG_ERROR(hr, fmt, ...) if (FAILED(hr))                                \
    log::instance().write_line(log_level::warning, fmt, __VA_ARGS__)

    // Cf. https://developer.nvidia.com/setstablepowerstateexe-%20disabling%20-gpu-boost-windows-10-getting-more-deterministic-timestamp-queries
    ATL::CComPtr<IDXGIFactory> factory;

    auto hr = ::CreateDXGIFactory1(IID_IDXGIFactory1, reinterpret_cast<void **>(
        &factory));
    LOG_ERROR(hr, "Failed to create DXGI factory for setting a stable power "
        "state on all GPUs.");

    for (UINT a = 0; SUCCEEDED(hr) || (hr != DXGI_ERROR_NOT_FOUND); ++a) {
        ATL::CComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC desc;
        ATL::CComPtr<ID3D12Device> device;

        hr = factory->EnumAdapters(a, &adapter);

        if (SUCCEEDED(hr)) {
            ::ZeroMemory(&desc, sizeof(desc));
            hr = adapter->GetDesc(&desc);
            LOG_ERROR(hr, "Failed to retrieve adapter description.");
            hr = S_OK;  // We ignore this error and output an empty name later.
        }

        if (SUCCEEDED(hr)) {
            hr = ::D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&device));
            LOG_ERROR(hr, "Failed to create Direct3D 12 device for graphics "
                "adapter \"{}\". This adapter will not be put into a stable "
                "power state.", W2A(desc.Description));
        }

        if (SUCCEEDED(hr)) {
            hr = device->SetStablePowerState(TRUE);
            LOG_ERROR(hr, "Failed to set a stable power state for graphics "
                "adapter \"{}\". Please make sure that you have installed "
                "the Windows 10 SDK and that your machine is in developer "
                "mode. The developer mode can be enabled in the update "
                "section of the Windows settings app.", W2A(desc.Description));
        }

        if (SUCCEEDED(hr)) {
            this->devices.push_back(std::move(device));
            log::instance().write_line(log_level::information, "The stable "
                "power state has been enabled for the graphics adapter "
                "\"{}\".", W2A(desc.Description));
        }
    }
#else  /* defined(TRROJAN_WITH_POWER_STATE_SCOPE) */
    log::instance().write_line(log_level::warning, "Enforcing a stable GPU "
        "power state requires Direct3D 12.");
#endif /* defined(TRROJAN_WITH_POWER_STATE_SCOPE) */
}


/*
 * trrojan::power_state_scope::~power_state_scope
 */
trrojan::power_state_scope::~power_state_scope(void) {
#if defined(TRROJAN_WITH_POWER_STATE_SCOPE)
    log::instance().write_line(log_level::information, "All graphics devices "
        "are leaving their stable power state.");
#endif /* defined(TRROJAN_WITH_POWER_STATE_SCOPE) */
}
