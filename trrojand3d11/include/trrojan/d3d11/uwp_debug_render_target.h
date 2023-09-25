/// <copyright file="debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>

#include <winrt/windows.ui.core.h>
#include <winrt/windows.graphics.display.h>

#include <atomic>
#include <memory>
#include <thread>

#include <atlbase.h>
#include <Windows.h>

#include "trrojan/d3d11/d2d_overlay.h"
#include "trrojan/d3d11/render_target.h"

#ifndef false//_UWP

/* Forward declatations. */
struct DebugConstants;


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The debug view is a render target using a visible window.
    /// </summary>
    class TRROJAND3D11_API uwp_debug_render_target : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        uwp_debug_render_target(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~uwp_debug_render_target(void);

        /// <inheritdoc />
        virtual void present(void);

        /// <inheritdoc />
        virtual void resize(const unsigned int width,
            const unsigned int height);

        /// <inheritdoc />
        virtual ATL::CComPtr<ID3D11UnorderedAccessView> to_uav(void);

        /// <summary>
        /// Interface to existing UWP window
        /// </summary>
        /// <param name="window"></param>
        void SetWindow(winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> const& window);

    private:

        typedef render_target_base base;

        // Cached reference to the Window.
        winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> m_window{ nullptr };

        // Cached device properties.
        winrt::Windows::Foundation::Size                       m_outputSize;
        winrt::Windows::Foundation::Size                       m_logicalSize;
        winrt::Windows::Graphics::Display::DisplayOrientations m_nativeOrientation;
        winrt::Windows::Graphics::Display::DisplayOrientations m_currentOrientation;
        float                                                  m_dpi;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        ATL::CComPtr<IDXGISwapChain1> swapChain;

        /// <summary>
        /// An unordered access view for compute shaders.
        /// </summary>
        /// <remarks>
        /// In contrast to the real benchmarking render target, the debug target
        /// uses an intermediate buffer (this one) and blits it on present. The
        /// reason for that is that mapping the back buffer is not recommended
        /// anymore and is also not supported on D3D12.
        /// </remarks>
        ATL::CComPtr<ID3D11UnorderedAccessView> _uav;

        /// Direct2D drawing components.
        winrt::com_ptr<ID2D1Factory3>       m_d2dFactory;
        winrt::com_ptr<ID2D1Device2>        m_d2dDevice;
        winrt::com_ptr<ID2D1DeviceContext2> m_d2dContext;
        winrt::com_ptr<ID2D1Bitmap1>        m_d2dTargetBitmap;

        std::unique_ptr<d2d_overlay> d2d_overlay_;

        // DirectWrite drawing components.
        winrt::com_ptr<IDWriteFactory3>		m_dwriteFactory;
        winrt::com_ptr<IWICImagingFactory2>	m_wicFactory;

        // Resources related to text rendering.
        std::wstring                            m_text;
        DWRITE_TEXT_METRICS	                    m_textMetrics;
        winrt::com_ptr<ID2D1SolidColorBrush>    m_whiteBrush;
        winrt::com_ptr<ID2D1DrawingStateBlock1> m_stateBlock;
        winrt::com_ptr<IDWriteTextLayout3>      m_textLayout;
        winrt::com_ptr<IDWriteTextFormat2>      m_textFormat;
    };
}
}


#endif