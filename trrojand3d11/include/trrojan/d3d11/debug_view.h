/// <copyright file="debug_view.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <atlbase.h>
#include <Windows.h>

#include "trrojan/d3d11/debugable.h"
#include "trrojan/d3d11/render_target.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The debug view is a render target with a window and a message pumping
    /// thread.
    /// </summary>
    class TRROJAND3D11_API debug_view : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        debug_view(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debug_view(void);

        /// <summary>
        /// Hides the debug window.
        /// </summary>
        inline void hide(void) {
            ::ShowWindow(this->hWnd, SW_HIDE);
        }

        /// <summary>
        /// Resize the render target to the given dimension.
        /// </summary>
        /// <remarks>
        /// The render target needs to be resized at least once to allocate
        /// all graphics resources. <see cref="run" /> and <see cref="start" />
        /// must not be called before this method was called the first time.
        /// </remarks>
        /// <param name="width"></param>
        /// <param name="const"></param>
        virtual void resize(const unsigned int width,
            const unsigned int height);

        /// <summary>
        /// Shows the debug window.
        /// </summary>
        void show(d3d11::debugable& content);

    private:

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
        /// Initialise window and swap chain.
        /// </summary>
        void initialise(void);

        /// <summary>
        /// Runs the message dispatcher.
        /// </summary>
        void message_loop(void);

        ATL::CComPtr<ID3D11Texture2D> content;

        /// <summary>
        /// The handle of the debug window.
        /// </summary>
        HWND hWnd;

        /// <summary>
        /// Determines whether <paramref name="msgPump" /> should run.
        /// </summary>
        std::atomic<bool> isRunning;

        /// <summary>
        /// The message pumping thread.
        /// </summary>
        std::thread msgPump;

        /// <summary>
        /// The pixel shader drawing the staged debug image.
        /// </summary>
        ATL::CComPtr<ID3D11PixelShader> pixelShader;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        ATL::CComPtr<IDXGISwapChain> swapChain;

        /// <summary>
        /// The vertex shader for drawing the screen-aligned quad.
        /// </summary>
        ATL::CComPtr<ID3D11VertexShader> vertexShader;
    };
}
}
