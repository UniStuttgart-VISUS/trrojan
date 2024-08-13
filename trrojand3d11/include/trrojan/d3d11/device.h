// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <d3d11_2.h>

#include <winrt/base.h>


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
        /// Gets the immediate context of the given device.
        /// </summary>
        /// <param name="device"></param>
        /// <returns></returns>
        static winrt::com_ptr<ID3D11DeviceContext> get_context(
            winrt::com_ptr<ID3D11Device> device);

        /// <summary>
        /// Gets the DXGI adapter for the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI adapter for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The adapter the device is running on.</returns>
        static winrt::com_ptr<IDXGIAdapter> get_dxgi_adapter(
            winrt::com_ptr<ID3D11Device> device);

        /// <summary>
        /// Gets the DXGI device for the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI device for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The underlying DXGI device.</returns>
        static winrt::com_ptr<IDXGIDevice> get_dxgi_device(
            winrt::com_ptr<ID3D11Device> device);

        /// <summary>
        /// Gets the DXGI factory that was used to create the given D3D device.
        /// </summary>
        /// <param name="device">The D3D device to get the DXGI factory for. It
        /// is safe to pass <c>nullptr</c>.</param>
        /// <returns>The underlying DXGI factory.</returns>
        static winrt::com_ptr<IDXGIFactory> get_dxgi_factory(
            winrt::com_ptr<ID3D11Device> device);

#if !defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d3d_device">The Direct3D device to be represented by
        /// this instance. This must not be <c>nullptr</c>.</param>
        explicit device(winrt::com_ptr<ID3D11Device> d3d_device);
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
        inline winrt::com_ptr<ID3D11DeviceContext>& d3d_context(void) {
#if defined(TRROJAN_FOR_UWP)
            if (this->_d3d_context == nullptr) {
                this->_d3d_context = get_context(this->d3d_device());
            }
#endif /* defined(TRROJAN_FOR_UWP) */
            return this->_d3d_context;
        }

        /// <summary>
        /// Answer the underlying Direct3D device.
        /// </summary>
        inline winrt::com_ptr<ID3D11Device> d3d_device(void) {
            return this->_d3d_device;
        }

#if defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Releases the underlying D3D device such that it must be recreated
        /// if it is needed again.
        /// </summary>
        inline void reset(void) {
            this->_d3d_context = nullptr;
            this->_d3d_device.reset(nullptr);
        }
#endif /* defined(TRROJAN_FOR_UWP) */

    private:

        void set_desc_from_device(void);

        winrt::com_ptr<ID3D11DeviceContext> _d3d_context;
#if defined(TRROJAN_FOR_UWP)
        lazy<winrt::com_ptr<ID3D11Device>> _d3d_device;
#else /* defined(TRROJAN_FOR_UWP) */
        winrt::com_ptr<ID3D11Device> _d3d_device;
#endif /* defined(TRROJAN_FOR_UWP) */

    };

} /* namespace d3d11 */
} /* namespace trrojan */

#include "trrojan/d3d11/device.inl"
