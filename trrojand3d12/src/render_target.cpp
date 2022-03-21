#if 0
// <copyright file="render_target.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>

#include "trrojan/d3d12/render_target.h"

#include <cassert>

#include "trrojan/image_helper.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"



/*
 * trrojan::d3d12::render_target_base::clear
 */
void trrojan::d3d12::render_target_base::clear(void) {
    assert(this->_device_context != nullptr);
    assert(this->_dsv != nullptr);
    assert(this->_rtv != nullptr);
    static const FLOAT CLEAR_COLOUR[] = { 0.0f, 0.0f, 0.0f, 0.0f }; // TODO
    this->_device_context->ClearRenderTargetView(this->_rtv, CLEAR_COLOUR);
    this->_device_context->ClearDepthStencilView(this->_dsv, D3D12_CLEAR_DEPTH,
        this->_depth_clear, 0);
}


/*
 * trrojan::d3d12::render_target_base::enable
 */
void trrojan::d3d12::render_target_base::enable(void) {
    assert(this->_device_context != nullptr);
    this->_device_context->OMSetDepthStencilState(this->_dss.p, 0);
    this->_device_context->OMSetRenderTargets(1, &this->_rtv.p, this->_dsv.p);
}


/*
 * trrojan::d3d12::render_target_base::present
 */
void trrojan::d3d12::render_target_base::present(void) { }


/*
 * trrojan::d3d12::render_target_base::save
 */
void trrojan::d3d12::render_target_base::save(const std::string& path) {
    if (this->_rtv != nullptr) {
        D3D12_TEXTURE2D_DESC desc;
        HRESULT hr = S_OK;
        ATL::CComPtr<ID3D12Resource> res;
        D3D12_MAPPED_SUBRESOURCE map;
        ATL::CComPtr<ID3D12Texture2D> tex;

        this->_rtv->GetResource(&res);
        hr = res->QueryInterface(&tex);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        tex->GetDesc(&desc);

        if (this->_staging_texture == nullptr) {
            
            desc.BindFlags = 0;
            desc.Usage = D3D12_USAGE_STAGING;
            desc.CPUAccessFlags = D3D12_CPU_ACCESS_READ;

            this->_staging_texture = nullptr;
            hr = this->device()->CreateTexture2D(&desc, nullptr,
                &this->_staging_texture);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
        assert(this->_staging_texture != nullptr);

        this->_device_context->CopyResource(this->_staging_texture, tex);

        hr = this->_device_context->Map(this->_staging_texture, 0,
            D3D12_MAP_READ, 0, &map);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        try {
            trrojan::wic_save(trrojan::get_wic_factory(), map.pData, desc.Width,
                desc.Height, map.RowPitch, GUID_WICPixelFormat32bppBGRA, path,
                GUID_NULL);
            this->_device_context->Unmap(this->_staging_texture, 0);
        } catch (...) {
            this->_device_context->Unmap(this->_staging_texture, 0);
            throw;
        }

    } else {
        log::instance().write_line(log_level::warning, "Benchmarking render "
            "target could not be saved, because it was not initialised.");
    }
}


/*
 * trrojan::d3d12::render_target_base::to_uav
 */
ATL::CComPtr<ID3D12UnorderedAccessView>
trrojan::d3d12::render_target_base::to_uav(void) {
    if (this->_rtv != nullptr) {
        ATL::CComPtr<ID3D12Resource> backBuffer;
        ATL::CComPtr<ID3D12Texture2D> texture;
        this->_rtv->GetResource(&backBuffer);
        this->_rtv = nullptr;
        backBuffer.QueryInterface(&texture);
        return create_uav(texture);

    } else {
        return nullptr;
    }
}

/*
 * trrojan::d3d12::render_target_base::use_reversed_depth_buffer
 */
void trrojan::d3d12::render_target_base::use_reversed_depth_buffer(
        const bool isEnabled) {
    this->_dss = nullptr;

    if (isEnabled) {
        D3D12_DEPTH_STENCIL_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));

        desc.DepthEnable = TRUE;
        desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D12_COMPARISON_GREATER;
        desc.StencilEnable = FALSE;

        auto hr = this->_device->CreateDepthStencilState(&desc, &this->_dss);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        this->_depth_clear = 0.0f;


    } else {
        this->_depth_clear = 1.0f;
    }

}

/*
 * trrojan::d3d12::render_target_base::render_target_base
 */
trrojan::d3d12::render_target_base::render_target_base(
        const trrojan::device& device) : _depth_clear(1.0f) {
    auto d = std::dynamic_pointer_cast<trrojan::d3d12::device>(device);
    if (d != nullptr) {
        this->_device = d->d3d_device();
    }
}



/*
 * trrojan::d3d12::render_target_base::set_back_buffer
 */
void trrojan::d3d12::render_target_base::set_back_buffer(
        ID3D12Texture2D *backBuffer) {
    assert(this->_dsv == nullptr);
    assert(this->_rtv == nullptr);
    assert(backBuffer != nullptr);
    ATL::CComPtr<ID3D12Texture2D> depthBuffer;
    HRESULT hr = S_OK;
    D3D12_TEXTURE2D_DESC texDesc;
    D3D12_VIEWPORT viewport;

    // Erase old staging texture for lazy recreate in save().
    this->_staging_texture = nullptr;

    hr = this->_device->CreateRenderTargetView(backBuffer, nullptr,
        &this->_rtv);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    backBuffer->GetDesc(&texDesc);
    texDesc.BindFlags = D3D12_BIND_DEPTH_STENCIL;
    texDesc.Format = DXGI_FORMAT_D32_FLOAT;

    hr = this->_device->CreateTexture2D(&texDesc, nullptr, &depthBuffer);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    hr = this->_device->CreateDepthStencilView(depthBuffer, nullptr,
            &this->_dsv);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(texDesc.Width);
    viewport.Height = static_cast<float>(texDesc.Height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    this->_device_context->RSSetViewports(1, &viewport);
}


/*
 * trrojan::d3d12::render_target_base::set_device
 */
void trrojan::d3d12::render_target_base::set_device(ID3D12Device *device) {
    assert(device != nullptr);
    this->_device = device;
}
#endif
