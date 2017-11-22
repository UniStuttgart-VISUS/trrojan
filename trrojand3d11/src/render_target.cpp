/// <copyright file="render_target.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/render_target.h"

#include <cassert>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::render_target_base::~render_target_base
 */
trrojan::d3d11::render_target_base::~render_target_base(void) { }


/*
 * trrojan::d3d11::render_target_base::clear
 */
void trrojan::d3d11::render_target_base::clear(void) {
    assert(this->_device_context != nullptr);
    assert(this->_dsv != nullptr);
    assert(this->_rtv != nullptr);
    static const FLOAT CLEAR_COLOUR[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    this->_device_context->ClearRenderTargetView(this->_rtv, CLEAR_COLOUR);
    this->_device_context->ClearDepthStencilView(this->_dsv, D3D11_CLEAR_DEPTH,
        1.0f, 0);
}


/*
 * trrojan::d3d11::render_target_base::enable
 */
void trrojan::d3d11::render_target_base::enable(void) {
    assert(this->_device_context != nullptr);
    this->_device_context->OMSetRenderTargets(1, &this->_rtv.p, this->_dsv.p);
}


/*
 * trrojan::d3d11::render_target_base::render_target_base
 */
trrojan::d3d11::render_target_base::render_target_base(
        const trrojan::device& device) {
    auto d = std::dynamic_pointer_cast<trrojan::d3d11::device>(device);
    if (d != nullptr) {
        this->_device = d->d3d_device();
        this->_device_context = d->d3d_context();
    }
}


/*
 * trrojan::d3d11::render_target_base::set_back_buffer
 */
void trrojan::d3d11::render_target_base::set_back_buffer(
        ID3D11Texture2D *backBuffer, bool createStagingTexture) {
    assert(this->_dsv == nullptr);
    assert(this->_rtv == nullptr);
    assert(backBuffer != nullptr);
    ATL::CComPtr<ID3D11Texture2D> depthBuffer;

    this->_staging_buffer = nullptr;
    if (createStagingTexture) {
        D3D11_TEXTURE2D_DESC texDesc;
        backBuffer->GetDesc(&texDesc);
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;
        texDesc.Usage = D3D11_USAGE_DEFAULT;

        auto hr = this->_device->CreateTexture2D(&texDesc, nullptr,
            &this->_staging_buffer);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->_device->CreateRenderTargetView(backBuffer, nullptr,
            &this->_rtv);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        D3D11_TEXTURE2D_DESC texDesc;
        backBuffer->GetDesc(&texDesc);
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        texDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

        auto hr = this->_device->CreateTexture2D(&texDesc, nullptr,
            &depthBuffer);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->_device->CreateDepthStencilView(depthBuffer, nullptr,
            &this->_dsv);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}


/*
 * trrojan::d3d11::render_target_base::set_device
 */
void trrojan::d3d11::render_target_base::set_device(ID3D11Device *device) {
    assert(device != nullptr);
    this->_device = device;
    this->_device_context == nullptr;
    this->_device->GetImmediateContext(&this->_device_context);
}
