// <copyright file="device.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <atlbase.h>
#include <d3d12.h>
#include <dxgi1_4.h>

#include "trrojan/device.h"

#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for devices to be tested.
    /// </summary>
    class TRROJAND3D12_API device : public trrojan::device_base {

    public:

        typedef std::shared_ptr<device> pointer;

        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d3dDevice">The Direct3D device to be represented by
        /// this instance. This must not be <c>nullptr</c>.</param>
        /// <param name="dxgiFactory">The DXGI factory used to obtain the
        /// underlying adapter. This is required due to the changes described
        /// on https://docs.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-1-4-improvements
        /// </param>
        device(const ATL::CComPtr<ID3D12Device>& d3dDevice,
            const ATL::CComPtr<IDXGIFactory4>& dxgiFactory);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device(void);

        ///// <summary>
        ///// Answer the immediate context of the underlying Direct3D device.
        ///// </summary>
        //inline ATL::CComPtr<ID3D12DeviceContext>& d3d_context(void) {
        //    return this->d3dContext;
        //}

        /// <summary>
        /// Answer the underlying Direct3D device.
        /// </summary>
        inline ATL::CComPtr<ID3D12Device>& d3d_device(void) {
            return this->_d3d_device;
        }

        /// <summary>
        /// Answer the underlying DXGI adapter on which the device was created.
        /// </summary>
        /// <returns></returns>
        ATL::CComPtr<IDXGIAdapter> dxgi_adapter(void);

        /// <summary>
        /// Answer the DXGI factory that was used to create the device.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<IDXGIFactory4>& dxgi_factory(void) {
            return this->_dxgi_factory;
        }

    private:

        //ATL::CComPtr<ID3D12DeviceContext> d3dContext;
        ATL::CComPtr<ID3D12Device> _d3d_device;
        ATL::CComPtr<IDXGIFactory4> _dxgi_factory;

    };
}
}
