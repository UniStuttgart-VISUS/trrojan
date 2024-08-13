// <copyright file="d2d_overlay.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/d2d_overlay.h"

#include <algorithm>
#include <system_error>

#include "trrojan/com_error_category.h"

#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::d2d_overlay::get_font
 */
winrt::com_ptr<IDWriteFont> trrojan::d3d11::d2d_overlay::get_font(
        IDWriteTextFormat *format) {
    BOOL exists = FALSE;
    winrt::com_ptr<IDWriteFontFamily> family;
    winrt::com_ptr<IDWriteFontCollection> fc;
    UINT32 index = 0;
    std::vector<wchar_t> name;
    winrt::com_ptr<IDWriteFont> retval;

    if (format == nullptr) {
        throw std::system_error(E_POINTER, com_category());
    }

    {
        auto cntBuffer = format->GetFontFamilyNameLength() + 1;
        name.resize(cntBuffer);
        auto hr = format->GetFontFamilyName(name.data(), cntBuffer);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = format->GetFontCollection(fc.put());
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = fc->FindFamilyName(name.data(), &index, &exists);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
        if (!exists) {
            throw std::system_error(E_FAIL, com_category());
        }
    }

    {
        auto hr = fc->GetFontFamily(index, family.put());
        if (FAILED(hr)) {
            throw std::system_error(E_FAIL, com_category());
        }
    }

    {
        auto hr = family->GetFirstMatchingFont(format->GetFontWeight(),
            format->GetFontStretch(), format->GetFontStyle(), retval.put());
        if (FAILED(hr)) {
            throw std::system_error(E_FAIL, com_category());
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::d2d_overlay::d2d_overlay
 */
trrojan::d3d11::d2d_overlay::d2d_overlay(ID3D11Device *device,
        IDXGISwapChain *swap_chain) {
    this->_d3d_device.copy_from(device);
    this->_swap_chain.copy_from(swap_chain);
    assert(this->_d3d_device != nullptr);
    assert(this->_swap_chain != nullptr);
    this->create_target_independent_resources();
    this->on_resized();
}


/*
 * trrojan::d3d11::d2d_overlay::begin_draw
 */
void trrojan::d3d11::d2d_overlay::begin_draw(void) {
    assert(this->_d2d_context != nullptr);
    assert(this->_drawing_state_block != nullptr);
    if (this->_d2d_context == nullptr) {
        throw std::system_error(E_NOT_VALID_STATE, com_category());
    }

    this->_d2d_context->SaveDrawingState(this->_drawing_state_block.get());
    this->_d2d_context->BeginDraw();
}


/*
 * trrojan::d3d11::d2d_overlay::create_brush
 */
winrt::com_ptr<ID2D1Brush> trrojan::d3d11::d2d_overlay::create_brush(
        const D2D1::ColorF& colour) {
    if (this->_d2d_context == nullptr) {
        throw std::system_error(E_NOT_VALID_STATE, com_category());
    }

    winrt::com_ptr<ID2D1Brush> retval;
    auto hr = this->_d2d_context->CreateSolidColorBrush(colour,
        reinterpret_cast<ID2D1SolidColorBrush **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d11::d2d_overlay::create_text_format
 */
winrt::com_ptr<IDWriteTextFormat> trrojan::d3d11::d2d_overlay::create_text_format(
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
            throw std::system_error(::GetLastError(), std::system_category());
        }

    } else {
        auto locale_len = ::wcslen(locale_name) + 1;
        locale.reserve(locale_len);
        std::copy(locale_name, locale_name + locale_len, locale.begin());
    }

    winrt::com_ptr<IDWriteTextFormat> retval;
    auto hr = this->_dwrite_factory->CreateTextFormat(font_family, NULL,
        font_weight, font_style, font_stretch, font_size, locale.data(),
        retval.put());
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());

    }

    return retval;
}


/*
 * trrojan::d3d11::d2d_overlay::draw_text
 */
void trrojan::d3d11::d2d_overlay::draw_text(const wchar_t *text,
        IDWriteTextFormat *format, ID2D1Brush *brush,
        const D2D1_RECT_F *layout_rect) {
    assert(this->_d2d_context != nullptr);
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

    this->_d2d_context->DrawText(text, static_cast<UINT32>(len), format, rect,
        brush);
}


/*
 * trrojan::d3d11::d2d_overlay::end_draw
 */
void trrojan::d3d11::d2d_overlay::end_draw(void) {
    assert(this->_d2d_context != nullptr);

    auto hr = _d2d_context->EndDraw();
    if (FAILED(hr) && (hr != D2DERR_RECREATE_TARGET)) {
        throw std::system_error(hr, com_category());
    }

    this->_d2d_context->RestoreDrawingState(this->_drawing_state_block.get());
}


/*
 * trrojan::d3d11::d2d_overlay::on_resize
 */
void trrojan::d3d11::d2d_overlay::on_resize(void) {
    this->release_target_dependent_resources();
}


/*
 * trrojan::d3d11::d2d_overlay::on_resized
 */
void trrojan::d3d11::d2d_overlay::on_resized(void) {
    this->create_target_dependent_resources();
}


/*
 * trrojan::d3d11::d2d_overlay::create_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_dependent_resources(
        winrt::com_ptr<IDXGISurface> surface) {
    assert(this->_d2d_context != nullptr);
    assert(surface != nullptr);

    {
    auto pixel_format = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,
        D2D1_ALPHA_MODE_PREMULTIPLIED);
        auto bmp_props = D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            pixel_format);

        // We force the target to be nullpt here, because during initial resize
        // of the window, the swap chain resize process is not involved.
        this->_d2d_target = nullptr;
        auto hr = this->_d2d_context->CreateBitmapFromDxgiSurface(
            surface.get(), &bmp_props, this->_d2d_target.put());
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }

        this->_d2d_context->SetTarget(this->_d2d_target.get());
    }

    this->_d2d_context->SetTextAntialiasMode(
        D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}


/*
 * trrojan::d3d11::d2d_overlay::create_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_dependent_resources(void) {
    assert(this->_d2d_context != nullptr);
    assert(this->_d2d_device != nullptr);
    assert(this->_d2d_factory != nullptr);
    assert(this->_d3d_device != nullptr);
    auto back_buffer = get_back_buffer(this->_swap_chain.get());
    this->create_target_dependent_resources(get_surface(back_buffer.get()));
}


/*
 * trrojan::d3d11::d2d_overlay::create_target_independent_resources
 */
void trrojan::d3d11::d2d_overlay::create_target_independent_resources(void) {
    assert(this->_d2d_context == nullptr);
    assert(this->_d2d_device == nullptr);
    assert(this->_d2d_factory == nullptr);
    assert(this->_depth_stencil_state == nullptr);
    assert(this->_dwrite_factory == nullptr);

    {
        auto hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
            ::IID_ID2D1Factory3,
            reinterpret_cast<void **>(&this->_d2d_factory));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto device = get_device(this->_d3d_device);
        auto hr = this->_d2d_factory->CreateDevice(device.get(),
            this->_d2d_device.put());
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = this->_d2d_device->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE, this->_d2d_context.put());
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = this->_d2d_factory->CreateDrawingStateBlock(
            this->_drawing_state_block.put());
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = ::DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(this->_dwrite_factory.get()));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }
}


/*
 * trrojan::d3d11::d2d_overlay::release_target_dependent_resources
 */
void trrojan::d3d11::d2d_overlay::release_target_dependent_resources(void) {
    this->_d2d_context->SetTarget(nullptr);
    this->_d2d_target = nullptr;
}
