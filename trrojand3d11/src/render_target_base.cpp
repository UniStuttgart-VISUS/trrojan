/// <copyright file="render_target_base" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/render_target_base.h"

#include <cassert>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::render_target_base::~render_target_base
 */
trrojan::d3d11::render_target_base::~render_target_base(void) { }


/*
 * trrojan::d3d11::render_target_base::resize
 */
void trrojan::d3d11::render_target_base::resize(const unsigned int width,
        const unsigned int height) {
    ATL::CComPtr<ID3D11Texture2D> backBuffer;

    // Make sure that alll existing views are released before resizing.
    this->dtv = nullptr;
    this->rtv = nullptr;

    if (this->swapChain == nullptr) {
        // Have no swap chain yet, so create one.
        if (this->device == nullptr) {
            throw std::runtime_error("A valid device must have been set before "
                "a render target can be resized.");
        }

        auto factory = device::get_dxgi_factory(this->device);
        if (factory == nullptr) {
            throw std::runtime_error("Cannot create swap chain without valid "
                "DXGI factory.");
        }

        DXGI_SWAP_CHAIN_DESC desc;
        ::memset(&desc, 0, sizeof(desc));
        desc.BufferCount = 2;
        desc.BufferDesc.Width = width;
        desc.BufferDesc.Height = height;
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = ::GetDesktopWindow();
        desc.Windowed = TRUE;

        auto hr = factory->CreateSwapChain(this->device, &desc,
            &this->swapChain);
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to create swap "
                "chain (error 0x%x).\n", hr);
            throw std::runtime_error("Failed to create swap chain.");
        }

    } else {
        // Resize existing swap chain.
        DXGI_SWAP_CHAIN_DESC desc;

        {
            auto hr = this->swapChain->GetDesc(&desc);
            if (FAILED(hr)) {
                log::instance().write(log_level::error, "Failed to retrieve "
                    "description of existing swap chain (error 0x%x).\n", hr);
                throw std::runtime_error("Failed to swap chain description.");
            }
        }

        {
            auto hr = this->swapChain->ResizeBuffers(desc.BufferCount,
                width, height, desc.BufferDesc.Format, 0);
            if (FAILED(hr)) {
                log::instance().write(log_level::error, "Failed to resize "
                    "swap chain (error 0x%x).\n", hr);
                throw std::runtime_error("Failed to resize swap chain.");
            }
        }
    }
    assert(this->swapChain != nullptr);

    {
        auto hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
            reinterpret_cast<void **>(&backBuffer));
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to retrieve back buffer from "
                "swap chain.");
        }
    }

    {
        auto hr = this->device->CreateRenderTargetView(backBuffer, nullptr,
            &this->rtv);
        if (FAILED(hr)) {
            throw std::runtime_error("Failed to create render target view.");
        }
    }
}


/*
 * trrojan::d3d11::render_target_base::render_target_base
 */
trrojan::d3d11::render_target_base::render_target_base(
        const trrojan::device& device) {
    auto d = std::dynamic_pointer_cast<trrojan::d3d11::device>(device);
    if (d != nullptr) {
        this->device = d->d3d_device();
        this->deviceContext = d->d3d_context();
    }
}
