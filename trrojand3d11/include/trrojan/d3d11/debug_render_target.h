#// <copyright file="debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <atlbase.h>
#include <Windows.h>

#include "trrojan/d3d11/d2d_overlay.h"
#include "trrojan/d3d11/render_target.h"

#ifndef _UWP

/* Forward declatations. */
struct DebugConstants;


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The debug view is a render target using a visible window.
    /// </summary>
    class TRROJAND3D11_API debug_render_target : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        debug_render_target(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debug_render_target(void);

        /// <inheritdoc />
        virtual void present(const UINT sync_interval) override;

        /// <inheritdoc />
        virtual void resize(const unsigned int width,
            const unsigned int height);

        /// <inheritdoc />
        virtual ATL::CComPtr<ID3D11UnorderedAccessView> to_uav(void);

    private:

        typedef render_target_base base;

        /// <summary>
        /// The name of the window class we are registering for the debug view.
        /// </summary>
        static const TCHAR *WINDOW_CLASS;

        /// <summary>
        /// The handler for window messages which makes the message pump exit if
        /// the escape key was pressed.
        /// </summary>
        static LRESULT WINAPI wndProc(HWND hWnd, UINT msg, WPARAM wParam,
            LPARAM lParam);

        /// <summary>
        /// Runs the message dispatcher.
        /// </summary>
        void doMsg(void);

        /// <summary>
        /// The handle of the debug window.
        /// </summary>
        std::atomic<HWND> hWnd;

        /// <summary>
        /// The message pumping thread.
        /// </summary>
        std::thread msgPump;

        /// <summary>
        /// A Direct2D/DirectWrite overlay for printing stupid messages.
        /// </summary>
        std::unique_ptr<d2d_overlay> _overlay;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        ATL::CComPtr<IDXGISwapChain> swapChain;

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
    };
}
}


#endif