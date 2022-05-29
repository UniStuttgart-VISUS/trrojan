// <copyright file="d2d_overlay.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/d2d_overlay.h"

#include <algorithm>
#include <vector>

#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/utilities.h"



/*
 * trrojan::d3d11::d2d_overlay::get_font
 */
ATL::CComPtr<IDWriteFont> trrojan::d3d11::d2d_overlay::get_font(
        IDWriteTextFormat *format) {
    BOOL exists = FALSE;
    ATL::CComPtr<IDWriteFontFamily> family;
    ATL::CComPtr<IDWriteFontCollection> fc;
    UINT32 index = 0;
    std::vector<wchar_t> name;
    ATL::CComPtr<IDWriteFont> retval;

    if (format == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    {
        auto cntBuffer = format->GetFontFamilyNameLength() + 1;
        name.resize(cntBuffer);
        auto hr = format->GetFontFamilyName(name.data(), cntBuffer);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = format->GetFontCollection(&fc);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = fc->FindFamilyName(name.data(), &index, &exists);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        if (!exists) {
            throw ATL::CAtlException(E_FAIL);
        }
    }

    {
        auto hr = fc->GetFontFamily(index, &family);
        if (FAILED(hr)) {
            throw ATL::CAtlException(E_FAIL);
        }
    }

    {
        auto hr = family->GetFirstMatchingFont(format->GetFontWeight(),
            format->GetFontStretch(), format->GetFontStyle(), &retval);
        if (FAILED(hr)) {
            throw ATL::CAtlException(E_FAIL);
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::d2d_overlay::d2d_overlay
 */
trrojan::d3d11::d2d_overlay::d2d_overlay(ID3D11Texture2D *texture)
        : _d3d_device(get_device(texture)), _texture(texture) {
    assert(this->_d3d_device != nullptr);
    assert(this->_texture != nullptr);
    set_debug_object_name(this->_texture.p, "D2D overlay buffer (externally "
        "provided)");
    this->create_target_independent_resources();
    this->create_target_dependent_resources();
    this->on_resized();
}


/*
 * trrojan::d3d11::d2d_overlay::d2d_overlay
 */
trrojan::d3d11::d2d_overlay::d2d_overlay(ID3D11Device *device,
        IDXGISwapChain *swap_chain)
    : _d3d_device(device),
        _texture(create_compatible_surface(device, swap_chain)) {
    assert(this->_d3d_device != nullptr);
    assert(this->_texture != nullptr);
    set_debug_object_name(this->_texture.p, "D2D overlay buffer");
    this->create_target_independent_resources();
    this->create_target_dependent_resources();
    this->on_resized();
}


/*
 * trrojan::d3d11::d2d_overlay::create_brush
 */
ATL::CComPtr<ID2D1Brush> trrojan::d3d11::d2d_overlay::create_brush(
        const D2D1::ColorF& colour) {
    if (this->_d2d_target == nullptr) {
        throw ATL::CAtlException(E_NOT_VALID_STATE);
    }

    ATL::CComPtr<ID2D1Brush> retval;
    auto hr = this->_d2d_target->CreateSolidColorBrush(colour,
        reinterpret_cast<ID2D1SolidColorBrush **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d11::d2d_overlay::create_text_format
 */
ATL::CComPtr<IDWriteTextFormat> trrojan::d3d11::d2d_overlay::create_text_format(
        const wchar_t *font_family,
        const float font_size,
        const DWRITE_FONT_WEIGHT font_weight,
        const DWRITE_FONT_STYLE font_style,
        const DWRITE_FONT_STRETCH font_stretch,
        const wchar_t *locale_name) {
    assert(this->_dwrite_factory != nullptr);

    std::vector<wchar_t> locale;    // The actual locale name.

    // If no locale is given, retrieve the name of the current user locale
    // and use this one. Otherwise, use the caller-defined locale name.
    if (locale_name == NULL) {
        auto locale_len = ::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,
            LOCALE_SNAME, NULL, 0);
        if (locale_len != 0) {
            locale.resize(locale_len);
            locale_len = ::GetLocaleInfoEx(LOCALE_NAME_USER_DEFAULT,
                LOCALE_SNAME, locale.data(), locale_len);
        }
        if (locale_len == 0) {
            throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
        }

    } else {
        auto locale_len = ::wcslen(locale_name) + 1;
        locale.reserve(locale_len);
        std::copy(locale_name, locale_name + locale_len, locale.begin());
    }

    ATL::CComPtr<IDWriteTextFormat> retval;
    auto hr = this->_dwrite_factory->CreateTextFormat(font_family, NULL,
        font_weight, font_style, font_stretch, font_size, locale.data(),
        &retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);

    }

    return retval;
}

/*
 * trrojan::d3d11::d2d_overlay::draw
 */
void trrojan::d3d11::d2d_overlay::draw(void) {
    assert(this->_d3d_device != nullptr);
    ATL::CComPtr<ID3D11DeviceContext> ctx;
    this->_d3d_device->GetImmediateContext(&ctx);

    // Configure input.
    ctx->IASetInputLayout(this->_input_layout);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    {
        ID3D11Buffer *vbs[] = { nullptr };
        UINT strides[] = { 0 };
        UINT offsets[] = { 0 };
        ctx->IASetVertexBuffers(0, 1, vbs, strides, offsets);
    }

    // Configure shaders.
    ctx->VSSetShader(this->_vs, nullptr, 0);
    ctx->DSSetShader(nullptr, nullptr, 0);
    ctx->HSSetShader(nullptr, nullptr, 0);
    ctx->GSSetShader(nullptr, nullptr, 0);
    ctx->PSSetShader(this->_ps, nullptr, 0);
    ctx->CSSetShader(nullptr, nullptr, 0);

    // Configure shader resources.
    ctx->PSSetShaderResources(0, 1, &this->_srv.p);
    ctx->PSSetSamplers(0, 1, &this->_sampler.p);

    // Configure rasteriser.
    ctx->RSSetState(this->_rasteriser_state);

    // Configure how output merger.
    ctx->OMSetDepthStencilState(this->_depth_stencil_state.p, 0);
    ctx->OMSetBlendState(this->_blend_state, nullptr, 0xffffffff);

    // Draw two triangles from nothing.
    ctx->Draw(4, 0);
}


/*
 * trrojan::d3d11::d2d_overlay::draw_text
 */
void trrojan::d3d11::d2d_overlay::draw_text(const wchar_t *text,
        IDWriteTextFormat *format, ID2D1Brush *brush,
        const D2D1_RECT_F *layout_rect) {
    assert(this->_d2d_target != nullptr);
    auto len = (text != nullptr) ? ::wcslen(text) : 0;
    D2D1_RECT_F rect;

    // Determine the layout rectangle as the whole render target if no
    // user-defined one was specified.
    if (layout_rect != nullptr) {
        rect = *layout_rect;
    } else {
        D2D1_SIZE_F size = this->_d2d_target->GetSize();
        rect = D2D1::RectF(0.0f, 0.0f, size.width, size.height);
    }

    //if (acquireMutexAndinitateDraw) {
    //    THE_DIRECTX_TRACE_INFO("Acquiring keyed mutex in "
    //        "dwrite_interop_surface::draw_text...\n");
    //    hr = this->acquire_direct2d_sync();
    //    if (FAILED(hr)) {
    //        THE_DIRECTX_TRACE_AND_THROW(hr, "Acquiring keyed mutex for "
    //            "Direct2D failed with error code %d.", hr);
    //    }

    //    renderTarget->BeginDraw();
    //} /* end if (acquireMutexAndinitateDraw) */

    this->_d2d_target->DrawText(text, static_cast<UINT32>(len), format, rect,
        brush);
}


/*
 * trrojan::d3d11::d2d_overlay::resize
 */
void trrojan::d3d11::d2d_overlay::resize(const UINT width, const UINT height) {
    // Preserve the texture description.
    assert(this->_texture != nullptr);
    D3D11_TEXTURE2D_DESC desc;
    this->_texture->GetDesc(&desc);

    // Release everything that is device dependent. This is necessary, because
    // for D3D-backed render targets, the D2D target cannot be resized, but must
    // be recreated. If the target is recreated, all resources need to be
    // recreated as well.
    this->on_resize();
    this->release_target_dependent_resources();

    // Resize the texture.
    desc.Width = width;
    desc.Height = height;

    this->_texture = nullptr;
    auto hr = this->_d3d_device->CreateTexture2D(&desc, nullptr,
        &this->_texture);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    // Reallocate resources, own and the ones of subclasses.
    this->create_target_dependent_resources();
    this->on_resized();
}


/*
 * trrojan::d3d11::d2d_overlay::on_resize
 */
void trrojan::d3d11::d2d_overlay::on_resize(void) { }


/*
 * trrojan::d3d11::d2d_overlay::on_resized
 */
void trrojan::d3d11::d2d_overlay::on_resized(void) { }


/*
 * trrojan::d3d11::d2d_overlay::create_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_dependent_resources(
        IDXGISurface *surface) {
    assert(surface != nullptr);
    assert(this->_d2d_factory != nullptr);
    assert(this->_d2d_target == nullptr);

    auto pixel_format = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,
        D2D1_ALPHA_MODE_PREMULTIPLIED);
    auto rt_props  = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT, pixel_format);

    {
        auto hr = this->_d2d_factory->CreateDxgiSurfaceRenderTarget(surface,
            &rt_props, &this->_d2d_target);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}


/*
 * trrojan::d3d11::d2d_overlay::create_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_dependent_resources(void) {
    assert(this->_d3d_device != nullptr);
    assert(this->_srv == nullptr);
    assert(this->_texture != nullptr);
    this->create_target_dependent_resources(get_surface(this->_texture));

    auto hr = this->_d3d_device->CreateShaderResourceView(this->_texture,
        nullptr, &this->_srv);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
}


/*
 * trrojan::d3d11::d2d_overlay::create_target_independent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_independent_resources(void) {
    assert(this->_blend_state == nullptr);
    assert(this->_d2d_factory == nullptr);
    assert(this->_depth_stencil_state == nullptr);
    assert(this->_dwrite_factory == nullptr);
    assert(this->_rasteriser_state == nullptr);

    {
        auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
            &this->_d2d_factory);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&this->_dwrite_factory));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    assert(this->_d3d_device != nullptr);
    {
        std::vector<uint8_t> src;
#ifndef _UWP
        src = d3d11::plugin::load_resource(MAKEINTRESOURCE(100),_T("SHADER"));
#else
        auto filepath = GetAppFolder().string() + "trrojand3d11" + "/" + "OverlayVertexShader.cso";
        src = ReadFileBytes(filepath);
#endif // !_UWP
        this->_vs = create_vertex_shader(this->_d3d_device, src);
        set_debug_object_name(this->_vs.p, "D2D overlay vertex shader");
        this->_input_layout = nullptr;
    }

    {
        std::vector<uint8_t> src;
#ifndef _UWP
        src = d3d11::plugin::load_resource(MAKEINTRESOURCE(101), _T("SHADER"));
#else
        auto filepath = GetAppFolder().string() + "trrojand3d11" + "/" + "OverlayPixelShader.cso";
        src = ReadFileBytes(filepath);
#endif // !_UWP
        this->_ps = create_pixel_shader(this->_d3d_device, src);
        set_debug_object_name(this->_ps.p, "D2D overlay pixel shader");
    }

    {
        D3D11_BLEND_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.RenderTarget[0].BlendEnable = TRUE;
        desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask
            = D3D11_COLOR_WRITE_ENABLE_ALL;

        auto hr = this->_d3d_device->CreateBlendState(&desc,
            &this->_blend_state);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        set_debug_object_name(this->_blend_state.p, "D2D overlay blend state");
    }

    {
        D3D11_RASTERIZER_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));

        // Default state works for now.
        //auto hr = this->_d3d_device->CreateRasterizerState(&desc,
        //    &this->_rasteriser_state);
        //if (FAILED(hr)) {
        //    throw ATL::CAtlException(hr);
        //}
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));

        desc.DepthEnable = FALSE;
        desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        desc.DepthFunc = D3D11_COMPARISON_GREATER;
        desc.StencilEnable = FALSE;

        auto hr = this->_d3d_device->CreateDepthStencilState(&desc,
            &this->_depth_stencil_state);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        set_debug_object_name(this->_depth_stencil_state.p, "D2D overlay "
            "depth/stencil state");
    }

    this->_sampler = create_linear_sampler(this->_d3d_device);
}


/*
 * trrojan::d3d11::d2d_overlay::release_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::release_target_dependent_resources(void) {
    this->_d2d_target = nullptr;
    this->_srv = nullptr;
}
