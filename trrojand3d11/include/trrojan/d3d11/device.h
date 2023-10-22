// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <d3d11_2.h>

#include "trrojan/device.h"
#include "trrojan/lazy.h"

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

#if !defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d3dDevice">The Direct3D device to be represented by
        /// this instance. This must not be <c>nullptr</c>.</param>
        explicit device(ATL::CComPtr<ID3D11Device> d3dDevice);
#endif /* !defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Initialises a new instance representing the D3D device created by
        /// the given generator function.
        /// </summary>
        /// <remarks>
        /// On the Xbox, we cannot have D3D11 and D3D12 device open at the same
        /// time. Therefore, we use lazy variables on UWP, which creates the
        /// device as needed and allows us to free it again when the environment
        /// is shut down.
        /// </remarks>
        /// <typeparam name="TGenerator">A functor type without a parameter that
        /// generates a D3D11 device.</typeparam>
        /// <param name="generator">The generator function for the device.
        /// </param>
        template<class TGenerator> explicit device(TGenerator&& generator);
#endif /* defined(TRROJAN_FOR_UWP) */

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device(void) = default;

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

#if defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Releases the underlying D3D device such that it must be recreated
        /// if it is needed again.
        /// </summary>
        inline void reset(void) {
            this->d3dContext.reset(nullptr);
            this->d3dDevice.reset(nullptr);
        }
#endif /* defined(TRROJAN_FOR_UWP) */

    private:

#if defined(TRROJAN_FOR_UWP)
        ATL::CComPtr<ID3D11DeviceContext> make_context(void);
#endif /* defined(TRROJAN_FOR_UWP) */

        void set_desc(void);

#if defined(TRROJAN_FOR_UWP)
        lazy<ATL::CComPtr<ID3D11DeviceContext>> d3dContext;
        lazy<ATL::CComPtr<ID3D11Device>> d3dDevice;
#else /* defined(TRROJAN_FOR_UWP) */
        ATL::CComPtr<ID3D11DeviceContext> d3dContext;
        ATL::CComPtr<ID3D11Device> d3dDevice;
#endif /* defined(TRROJAN_FOR_UWP) */

    };

} /* namespace d3d11 */
} /* namespace trrojan */

#include "trrojan/d3d11/device.inl"
