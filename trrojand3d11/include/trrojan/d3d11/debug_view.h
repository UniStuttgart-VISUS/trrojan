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


/* Forward declatations. */
struct DebugConstants;


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The debug view is a render target with a window and a message pumping
    /// thread.
    /// </summary>
    class TRROJAND3D11_API debug_view {

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
        void show(debugable& content);

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

        void set_rtv(ID3D11Texture2D *texture);

        /// <summary>
        /// GPU buffer for <see cref="constants" />.
        /// </summary>
        ATL::CComPtr<ID3D11Buffer> cbConstants;

        /// <summary>
        /// The per-frame constants of the debug view.
        /// </summary>
        std::unique_ptr<DebugConstants> constants;

        /// <summary>
        /// Immediate context of <see cref="debug_view::device" />.
        /// </summary>
        ATL::CComPtr<ID3D11DeviceContext> context;

        /// <summary>
        /// Lock for the shared resource beneath
        /// <see cref="debug_view::contentView" />.
        /// </summary>
        ATL::CComPtr<IDXGIKeyedMutex> contentLock;

        /// <summary>
        /// Shader resource view for the content texture.
        /// </summary>
        ATL::CComPtr<ID3D11ShaderResourceView> contentView;

        /// <summary>
        /// Depth/stencil state disabling depth test.
        /// </summary>
        ATL::CComPtr<ID3D11DepthStencilState> depthState;

        /// <summary>
        /// The device used for rendering the debug view.
        /// </summary>
        ATL::CComPtr<ID3D11Device> device;

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
        /// The rasteriser state of the debugging renderer.
        /// </summary>
        ATL::CComPtr<ID3D11RasterizerState> rasteriserState;

        /// <summary>
        /// The render target view.
        /// </summary>
        ATL::CComPtr<ID3D11RenderTargetView> rtv;

        /// <summary>
        /// A linear sampler.
        /// </summary>
        ATL::CComPtr<ID3D11SamplerState> samplerState;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        ATL::CComPtr<IDXGISwapChain> swapChain;

        /// <summary>
        /// The vertex shader for drawing the screen-aligned quad.
        /// </summary>
        ATL::CComPtr<ID3D11VertexShader> vertexShader;

        /// <summary>
        /// The viewport of the debug view.
        /// </summary>
        D3D11_VIEWPORT viewport;
    };
}
}
