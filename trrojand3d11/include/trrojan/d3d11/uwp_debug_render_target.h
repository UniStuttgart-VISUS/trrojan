// <copyright file="debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>
// <author>Christoph Müller</author>

#pragma once

#include <atlbase.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <Windows.h>

#include "trrojan/uwp_render_target_base.h"

#if defined(CREATE_D2D_OVERLAY)
#include "trrojan/d3d11/d2d_overlay.h"
#endif // defined(CREATE_D2D_OVERLAY)

#include "trrojan/d3d11/render_target.h"


namespace trrojan {
namespace d3d11 {


#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// The debug view is a render target using a visible window.
    /// </summary>
    class TRROJAND3D11_API uwp_debug_render_target
            : public uwp_render_target_base<render_target_base> {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        uwp_debug_render_target(void) = default;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~uwp_debug_render_target(void);

        /// <inheritdoc />
        virtual void present(void);

        virtual void present(const UINT sync_interval) {
            // TODO: massive hack.
            this->present();
        }

        /// <inheritdoc />
        virtual void resize(const unsigned int width,
            const unsigned int height);

        /// <inheritdoc />
        virtual ATL::CComPtr<ID3D11UnorderedAccessView> to_uav(void);

    private:

        typedef uwp_render_target_base<render_target_base> base;

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
#if defined(CREATE_D2D_OVERLAY)
        std::unique_ptr<d2d_overlay> d2d_overlay_;
#endif // defined(CREATE_D2D_OVERLAY)

        winrt::com_ptr<ID2D1Factory3>       m_d2dFactory;
        winrt::com_ptr<ID2D1Device2>        m_d2dDevice;
        winrt::com_ptr<ID2D1DeviceContext2> m_d2dContext;
        winrt::com_ptr<ID2D1Bitmap1>        m_d2dTargetBitmap;

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
#endif /* defined(TRROJAN_FOR_UWP) */

} /* namespace d3d11 */
} /* namespace trrojan */
