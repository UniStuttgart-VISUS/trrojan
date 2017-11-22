/// <copyright file="render_target.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

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

        /// <summary>
        /// Clears the render target.
        /// </summary>
        /// <remarks>
        /// A device must have been set and the render target must have been
        /// resized at least once before this method can be called.
        /// </remarks>
        void clear(void);

        /// <summary>
        /// Answer the device the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D11Device>& device(void) {
            return this->_device;
        }

        /// <summary>
        /// Answer the device context belonging to
        /// <see cref="render_target_base::device" />.
        /// </summary>
        /// <returns></returns>
        inline ATL::CComPtr<ID3D11DeviceContext> device_context(void) {
            return this->_device_context;
        }

        /// <summary>
        /// Sets the render target as active target of the output merger of its
        /// associated device.
        /// </summary>
        void enable(void);

        /// <summary>
        /// Resizes the swap chain of the render target to the given dimension.
        /// </summary>
        /// <param name="width">The new width of the render target in pixels.
        /// </param>
        /// <param name="height">The new height of the render target in pixels.
        /// </param>
        virtual void resize(const unsigned int width,
            const unsigned int height) = 0;

        /// <summary>
        /// Answer, if any, the staging buffer used to share the results with an
        /// on-screen view.
        /// </summary>
        inline ATL::CComPtr<ID3D11Texture2D> staging_buffer(void) {
            return this->_staging_buffer;
        }

        render_target_base& operator =(const render_target_base&) = delete;

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="device">The device to create the target on.</param>
        render_target_base(const trrojan::device& device);

        /// <summary>
        /// Set <paramref name="backBuffer" /> as back buffer and creates the
        /// render target view for it. Furthermore, allocate a depth buffer of
        /// the same size and create the view for it.
        /// </summary>
        /// <remarks>
        /// <see cref="_dsv" /> and <see cref="_rtv" /> must have been deleted
        /// before this method can be called.
        /// </remarks>
        void set_back_buffer(ID3D11Texture2D *backBuffer,
            bool createStagingTexture = false);

        /// <summary>
        /// Sets the D3D device and updates the immediate context.
        /// </summary>
        void set_device(ID3D11Device *device);

        /// <summary>
        /// The depth/stencil view.
        /// </summary>
        ATL::CComPtr<ID3D11DepthStencilView> _dsv;

        /// <summary>
        /// The render target view.
        /// </summary>
        ATL::CComPtr<ID3D11RenderTargetView> _rtv;

    private:

        /// <summary>
        /// The device the render target lives on.
        /// </summary>
        ATL::CComPtr<ID3D11Device> _device;

        /// <summary>
        /// The immediate context of <see cref="_device" />.
        /// </summary>
        ATL::CComPtr<ID3D11DeviceContext> _device_context;

        /// <summary>
        /// Holds a staging buffer for sharing the back buffer.
        /// </summary>
        ATL::CComPtr<ID3D11Texture2D> _staging_buffer;
    };


    /// <summary>
    /// Pointer to a generic render target.
    /// </summary>
    typedef std::shared_ptr<render_target_base> render_target;
}
}
