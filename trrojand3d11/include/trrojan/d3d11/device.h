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
    class TRROJAND3D11_API device : trrojan::device_base {

    public:

        typedef std::shared_ptr<device> pointer;

        /// <summary>
        /// Initialises a new instance representing the given D3D device.
        /// </summary>
        /// <param name="d">The Direct3D device to be represented by this
        /// instance. This must not be <c>nullptr</c>.</param>
        explicit device(ATL::CComPtr<ID3D11Device> d);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~device(void);

    private:

        ATL::CComPtr<ID3D11Device> _device;

    };
}
}
