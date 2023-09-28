//// <copyright file="uwp_debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>

#include <atomic>
#include <memory>
#include <thread>

#include <winrt/windows.graphics.display.h>

#include <atlbase.h>
#include <Windows.h>

#if defined(CREATE_D2D_OVERLAY)
#include "trrojan/d3d12/d2d_overlay.h"
#endif // defined(CREATE_D2D_OVERLAY)

#include "trrojan/d3d12/render_target.h"

#ifdef _UWP

/* Forward declarations. */
struct DebugConstants;


namespace trrojan {
    namespace d3d12 {

        /// <summary>
        /// The debug view is a render target using a visible window.
        /// </summary>
        /// <remarks>
        /// This debug render target works by copying the data from a UAV that is
        /// used as the actual render target. This way, it is ensured that we
        /// actually see what the real thing is doing rather than having a
        /// completely different setup that might hide or induce undesired effects.
        /// </remarks>
        class TRROJAND3D12_API uwp_debug_render_target : public render_target_base {

        public:

            /// <summary>
            /// Initialises a new instance.
            /// </summary>
            uwp_debug_render_target(const trrojan::device& device);

            /// <summary>
            /// Finalises the instance.
            /// </summary>
            virtual ~uwp_debug_render_target(void);

            /// <inheritdoc />
            UINT present(void) override;

            // <inheritdoc />
            void resize(const unsigned int width,
                const unsigned int height) override;

            ///// <inheritdoc />
            //void to_uav(const D3D12_CPU_DESCRIPTOR_HANDLE dst,
            //    ID3D12GraphicsCommandList *cmd_list) override;

            /// <summary>
            /// Interface to existing UWP window
            /// </summary>
            /// <param name="window"></param>
            void SetWindow(winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> const& window);

        protected:

            /// <inheritdoc />
            void reset_buffers(void) override;

        private:

            typedef render_target_base base;

            // Cached reference to the Window.
            winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> window_;

            // Cached device properties.
            winrt::Windows::Foundation::Size                       output_size_;
            winrt::Windows::Foundation::Size                       logical_size_;
            winrt::Windows::Graphics::Display::DisplayOrientations native_orientation_;
            winrt::Windows::Graphics::Display::DisplayOrientations current_orientation_;
            float                                                  dpi_;

            /// <summary>
            /// The staging buffer used to back the UAV that we provide to external
            /// users for writing to the debug render target. If the render target
            /// is enabled, this staging buffer will actually become the target.
            /// Before presenting the debug target, its content will be copied to
            /// the back buffer of the swap chain.
            /// </summary>
            ATL::CComPtr<ID3D12Resource> staging_buffer_;

            /// <summary>
            /// The swap chain for the window.
            /// </summary>
            ATL::CComPtr<IDXGISwapChain3> swap_chain_;

            /// Direct2D drawing components
#if defined(CREATE_D2D_OVERLAY)
            std::unique_ptr<d2d_overlay> d2d_overlay_;
#endif // defined(CREATE_D2D_OVERLAY)

            winrt::com_ptr<ID2D1Factory3>       d2d_factory_;
            winrt::com_ptr<ID2D1Device2>        d2d_device_;
            winrt::com_ptr<ID2D1DeviceContext2> d2d_device_context_;
            winrt::com_ptr<ID2D1Bitmap1>        d2d_target_bitmap_;

            // DirectWrite drawing components
            winrt::com_ptr<IDWriteFactory3>     dwrite_factory_;
            winrt::com_ptr<IWICImagingFactory2> wic_factory_;

            // Resources related to text rendering
            std::wstring                            text_;
            DWRITE_TEXT_METRICS                     text_metrics_;
            winrt::com_ptr<ID2D1SolidColorBrush>    white_brush_;
            winrt::com_ptr<ID2D1DrawingStateBlock1> state_block_;
            winrt::com_ptr<IDWriteTextLayout3>      text_layout_;
            winrt::com_ptr<IDWriteTextFormat2>      text_format_;
        };
    }
}

#endif // _UWP