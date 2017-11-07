/// <copyright file="debugable.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <Windows.h>
#include <atlbase.h>
#include <d3d11.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Defines the interface for classes which have debug rendering
    /// capabilities.
    /// </summary>
    class TRROJAND3D11_API debugable {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debugable(void) = default;

        /// <summary>
        /// This method requests the object to draw its debug view using
        /// the specified device and its immediate context.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="deviceContext"></param>
        virtual void draw_debug_view(ATL::CComPtr<ID3D11Device> device,
            ATL::CComPtr<ID3D11DeviceContext> deviceContext) = 0;

        /// <summary>
        /// This method is invoked if the debug view was resized to the given
        /// dimensions.
        /// </summary>
        /// <remarks>
        /// Allocate all device resources used for debug rendering in this
        /// method.
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="width"></param>
        /// <param name="height"></param>
        virtual void on_debug_view_resized(ATL::CComPtr<ID3D11Device> device,
            const unsigned int width, const unsigned int height) = 0;

        /// <summary>
        /// This method is invoked before the debug view is being resized.
        /// </summary>
        /// <remarks>
        /// Release all device resources used for debug rendering in this
        /// method.
        /// </remarks>
        virtual void on_debug_view_resizing(void) = 0;

    protected:

        /// <summary>
        /// Disallow direct instances of the class.
        /// </summary>
        debugable(void) = default;

    };
}
}
