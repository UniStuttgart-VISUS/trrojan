/// <copyright file="device.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <d3d11.h>

#include "trrojan/device.h"

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for devices to be tested.
    /// </summary>
    class TRROJAND3D11_API device : public trrojan::device_base {

    public:

        typedef std::shared_ptr<device> pointer;

        /// <summary>
        /// Gets the DXGI adapter for the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI adapter for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The adapter the device is running on.</returns>
        static ATL::CComPtr<IDXGIAdapter> get_dxgi_adapter(
            ATL::CComPtr<ID3D11Device> device);

        /// <summary>
        /// Gets the DXGI device for the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI device for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The underlying DXGI device.</returns>
        static ATL::CComPtr<IDXGIDevice> get_dxgi_device(
            ATL::CComPtr<ID3D11Device> device);

        /// <summary>
        /// Gets the DXGI factory that was used to create the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI factory for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The underlying DXGI factory.</returns>
        static ATL::CComPtr<IDXGIFactory> get_dxgi_factory(
            ATL::CComPtr<ID3D11Device> device);

        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d3dDevice">The Direct3D device to be represented by
        /// this instance. This must not be <c>nullptr</c>.</param>
        explicit device(ATL::CComPtr<ID3D11Device> d3dDevice);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device(void);

        /// <summary>
        /// Answer the immediate context of the underlying Direct3D device.
        /// </summary>
        inline ATL::CComPtr<ID3D11DeviceContext>& d3d_context(void) {
            return this->d3dContext;
        }

        /// <summary>
        /// Answer the underlying Direct3D device.
        /// </summary>
        inline ATL::CComPtr<ID3D11Device>& d3d_device(void) {
            return this->d3dDevice;
        }

    private:

        ATL::CComPtr<ID3D11DeviceContext> d3dContext;
        ATL::CComPtr<ID3D11Device> d3dDevice;

    };
}
}
