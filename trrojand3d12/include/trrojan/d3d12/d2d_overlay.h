﻿// <copyright file="d2d_overlay.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <Windows.h>
#include <d2d1_3.h>
#include <d3d11on12.h>
#include <dxgi1_4.h>
#include <dwrite.h>

#include <winrt/base.h>

#include "trrojan/device.h"

#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A D2D render target that can be used to produce a 2D overlay on a
    /// DXGI swap chain.
    /// </summary>
    /// <remarks>
    /// <para>There are two ways of using this class: The first one is
    /// instantiating it and calling either the convenience methods for
    /// creating resources and drawing text or issuing custom draw
    /// commands via the render target exposed by the overlay.</para>
    /// <para>The second way is deriving a custom overlay from this
    /// class. This method has the advantage that overriding the two
    /// methods <see cref="on_resize" /> and <see cref="on_resized" />
    /// facilitates the life-time management for Direct2D and DirectWrite
    /// resources.</para>
    /// </remarks>
    class TRROJAND3D12_API d2d_overlay {

    public:

        /// <summary>
        /// Gets the font used in the given text format.
        /// </summary>
        /// <remarks>
        /// This information might be valuable to compute positions and sizes
        /// depending on the font metrics.
        /// </remarks>
        /// <param name="format"></param>
        /// <returns></returns>
        static winrt::com_ptr<IDWriteFont> get_font(IDWriteTextFormat *format);

        /// <summary>
        /// Initialises an overlay for the given swap chain.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="swap_chain"></param>
        d2d_overlay(
            winrt::com_ptr<ID3D12Device> device,
            winrt::com_ptr<ID3D12CommandQueue> command_queue,
            winrt::com_ptr<IDXGISwapChain3> swap_chain,
            UINT frame_count
        );

        d2d_overlay(const d2d_overlay& rhs) = delete;

        virtual ~d2d_overlay(void) = default;

        /// <summary>
        /// Begin drawing 2D content.
        /// </summary>
        /// <param name="frame_index"></param>
        void begin_draw(UINT frame_index);

        /// <summary>
        /// Clears the 2D content.
        /// </summary>
        /// <param name=""></param>
        inline void clear(void) {
            assert(this->_d2d_context != nullptr);
            this->_d2d_context->Clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
        }

        /// <summary>
        /// Create a solid brush of the given colour.
        /// </summary>
        /// <param name="colour"></param>
        /// <returns></returns>
        winrt::com_ptr<ID2D1Brush> create_brush(const D2D1::ColorF& colour);

        /// <summary>
        /// Creates a text format.
        /// </summary>
        /// <param name="font_family"></param>
        /// <param name="font_size"></param>
        /// <param name="font_weight"></param>
        /// <param name="font_style"></param>
        /// <param name="font_stretch"></param>
        /// <param name="locale_name"></param>
        /// <returns></returns>
        winrt::com_ptr<IDWriteTextFormat> create_text_format(
            const wchar_t *font_family, const float font_size,
            const DWRITE_FONT_WEIGHT font_weight = DWRITE_FONT_WEIGHT_NORMAL,
            const DWRITE_FONT_STYLE font_style = DWRITE_FONT_STYLE_NORMAL,
            const DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL,
            const wchar_t *locale_name = nullptr);

        /// <summary>
        /// Draws text on the overlay.
        /// </summary>
        /// <param name="text"></param>
        /// <param name="format"></param>
        /// <param name="brush"></param>
        /// <param name="layout_rect"></param>
        void draw_text(const wchar_t *text, IDWriteTextFormat *format,
            ID2D1Brush *brush, const D2D1_RECT_F *layout_rect = nullptr);

        /// <summary>
        /// Daws a text using a transient <see cref="IDWriteTextFormat" /> for
        /// formatting.
        /// </summary>
        /// <remarks>
        /// This method is not recommended for production use. Allocate and
        /// reuse graphics resources as long as the render target has not been
        /// invalidated.
        /// </remarks>
        /// <param name="text"></param>
        /// <param name="font_family"></param>
        /// <param name="font_size"></param>
        /// <param name="brush"></param>
        /// <param name="font_weight"></param>
        /// <param name="font_style"></param>
        /// <param name="font_stretch"></param>
        /// <param name="layout_rect"></param>
        /// <param name="locale_name"></param>
        inline void draw_text(const wchar_t *text, const wchar_t *font_family,
                const float font_size,
                ID2D1Brush *brush,
                const DWRITE_FONT_WEIGHT font_weight = DWRITE_FONT_WEIGHT_NORMAL,
                const DWRITE_FONT_STYLE font_style = DWRITE_FONT_STYLE_NORMAL,
                const DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL,
                const D2D1_RECT_F *layout_rect = nullptr,
                const wchar_t *locale_name = nullptr) {
            auto format = this->create_text_format(font_family, font_size,
                font_weight, font_style, font_stretch, locale_name);
            this->draw_text(text, format.get(), brush, layout_rect);
        }

        /// <summary>
        /// Draws a text using a transient <see cref="IDWriteTextFormat" /> and
        /// <see cref="ID2D1Brush" /> for rendering.
        /// </summary>
        /// <remarks>
        /// This method is not recommended for production use. Allocate and
        /// reuse graphics resources as long as the render target has not been
        /// invalidated.
        /// </remarks>
        /// <param name="text"></param>
        /// <param name="font_family"></param>
        /// <param name="font_size"></param>
        /// <param name="colour"></param>
        /// <param name="font_weight"></param>
        /// <param name="font_style"></param>
        /// <param name="font_stretch"></param>
        /// <param name="layout_rect"></param>
        /// <param name="locale_name"></param>
        inline void draw_text(const wchar_t *text, const wchar_t *font_family,
                const float font_size, const D2D1::ColorF colour,
                const DWRITE_FONT_WEIGHT font_weight = DWRITE_FONT_WEIGHT_NORMAL,
                const DWRITE_FONT_STYLE font_style = DWRITE_FONT_STYLE_NORMAL,
                const DWRITE_FONT_STRETCH font_stretch = DWRITE_FONT_STRETCH_NORMAL,
                const D2D1_RECT_F *layout_rect = nullptr,
                const wchar_t *locale_name = nullptr) {
            auto format = this->create_text_format(font_family, font_size,
                font_weight, font_style, font_stretch, locale_name);
            auto brush = this->create_brush(colour);
            this->draw_text(text, format.get(), brush.get(), layout_rect);
        }

        /// <summary>
        /// End drawing 2D content.
        /// </summary>
        void end_draw(void);

        /// <summary>
        /// Gets the Direct2D factory used to create the overlay.
        /// </summary>
        /// <returns></returns>
        inline winrt::com_ptr<ID2D1Factory3> get_d2d_factory(void) const {
            return this->_d2d_factory;
        }

        /// <summary>
        /// Gets the DirectWrite factory used to render text on the overlay.
        /// </summary>
        /// <returns></returns>
        inline winrt::com_ptr<IDWriteFactory> get_dwrite_factory(void) const {
            return this->_dwrite_factory;
        }

        /// <summary>
        /// Releases all resources depending on the render target/swap chain
        /// before the swap chain is resized.
        /// </summary>
        /// <remarks>
        /// Subclasses must release all resources depending on the render target
        /// in this method. Subclasses must make sure to call the parent
        /// implementation.
        /// </remarks>
        virtual void on_resize(void);

        /// <summary>
        /// (Re-) Creates rendering resources after the swap chain was resized.
        /// </summary>
        /// <remarks>
        /// Subclasses must (re-) create all resources depending on the render
        ///  target in this method. Subclasses must make sure to call the parent
        /// implementation.
        /// </remarks>
        virtual void on_resized(void);

        /// <summary>
        /// Sets the 2D transformation to be the identity matrix.
        /// Currently not in use.
        /// </summary>
        void set_identity_transform(UINT frame_index) {
            assert(this->_d2d_render_targets[frame_index] != nullptr);
            this->_d2d_context->SetTransform(D2D1::IdentityMatrix());
        }

        d2d_overlay& operator =(const d2d_overlay& rhs) = delete;

        /// <summary>
        /// Gets the Direct2D context.
        /// </summary>
        inline operator winrt::com_ptr<ID2D1DeviceContext2>(void) {
            return this->_d2d_context;
        }

    private:

        void create_target_dependent_resources(IDXGISurface *surface, int i);

        void create_target_dependent_resources(void);

        void create_target_independent_resources(void);

        void release_target_dependent_resources(void);

        winrt::com_ptr<ID2D1DeviceContext2> _d2d_context;
        winrt::com_ptr<ID2D1Device2> _d2d_device;
        winrt::com_ptr<ID2D1Factory3> _d2d_factory;
        std::vector<winrt::com_ptr<ID2D1Bitmap1>> _d2d_render_targets;

        winrt::com_ptr<ID3D12Device> _d3d12_device;
        winrt::com_ptr<ID3D12CommandQueue> _d3d12_command_queue;
        winrt::com_ptr<ID3D11On12Device> _d3d11on12_device;
        winrt::com_ptr<ID3D11DeviceContext> _d3d11_device_context;

        // currently removed since it is not used anyway
        // winrt::com_ptr<ID3D12DepthStencilState> _depth_stencil_state;
        winrt::com_ptr<ID2D1DrawingStateBlock1> _drawing_state_block;
        winrt::com_ptr<IDWriteFactory> _dwrite_factory;

        winrt::com_ptr<IDXGISwapChain3> _swap_chain;
        std::vector<winrt::com_ptr<ID3D12Resource>> _render_targets;
        std::vector<winrt::com_ptr<ID3D11Resource>> _wrapped_back_buffers;
        UINT _frame_count;
        UINT _current_frame;
    };
}
}
