/// <copyright file="debug_render_target.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/debug_render_target.h"

#include <cassert>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::debug_render_target::~debug_render_target
 */
trrojan::d3d11::debug_render_target::~debug_render_target(void) { }


/*
 * trrojan::d3d11::debug_render_target::resize
 */
void trrojan::d3d11::debug_render_target::resize(const unsigned int width,
        const unsigned int height) {
    if (this->swapChain == nullptr) {
        assert(this->device == nullptr);
        assert(this->deviceContext == nullptr);

        auto hr = ::D3D11CreateDeviceAndSwapChain(nullptr,
            D3D_DRIVER_TYPE_HARDWARE, NULL, 0, nullptr, 0, D3D11_SDK_VERSION,
            nullptr, &this->swapChain, &this->device, nullptr,
            &this->deviceContext);
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to create Direct3D "
                "11 with swap chain (error 0x%x).\n", hr);
            throw std::runtime_error("Failed to create device and swap chain.");
        }


    } else {
        render_target_base::resize(width, height);
    }

}
