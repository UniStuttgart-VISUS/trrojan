/// <copyright file="render_target_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <d3d11.h>

#include "trrojan/d3d11/device.h"
#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for production and debug render targets.
    /// </summary>
    class TRROJAND3D11_API render_target_base {

    public:

        render_target_base(const render_target_base&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~render_target_base(void);

        render_target_base& operator =(const render_target_base&) = delete;

        /// <summary>
        /// Resizes the swap chain of the render target to the given dimension.
        /// </summary>
        /// <param name="width">The new width of the render target in pixels.
        /// </param>
        /// <param name="height">The new height of the render target in pixels.
        /// </param>
        virtual void resize(const unsigned int width,
            const unsigned int height);

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        render_target_base(const trrojan::device& device);

        ATL::CComPtr<ID3D11DepthStencilView> dtv;
        ATL::CComPtr<ID3D11Device> device;
        ATL::CComPtr<ID3D11DeviceContext> deviceContext;
        ATL::CComPtr<ID3D11RenderTargetView> rtv;
        ATL::CComPtr<IDXGISwapChain> swapChain;

    };
}
}