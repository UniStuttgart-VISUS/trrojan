﻿// <copyright file="render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>
#include <d3d11.h>

#include <winrt/base.h>

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
        virtual void clear(void);

        /// <summary>
        /// Answer the device the render target belongs to.
        /// </summary>
        /// <returns></returns>
        inline winrt::com_ptr<ID3D11Device>& device(void) {
            return this->_device;
        }

        /// <summary>
        /// Answer the device context belonging to
        /// <see cref="render_target_base::device" />.
        /// </summary>
        /// <returns></returns>
        inline winrt::com_ptr<ID3D11DeviceContext> device_context(void) {
            return this->_device_context;
        }

        /// <summary>
        /// Sets the render target as active target of the output merger of its
        /// associated device.
        /// </summary>
        void enable(void);

        /// <summary>
        /// Performs cleanup operations once a frame was completed.
        /// </summary>
        /// <remarks>
        /// The default implementation does nothing.
        /// </remarks>
        /// <param name="sync_interval">Zero for immediately presentation, N for
        /// synchronising on the Nth vblank. The parameter has only an effect on
        /// the debug render target that has an actual swap chain.
        /// </param>
        virtual void present(const UINT sync_interval);

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
        /// Stage the current back buffer and save it to the given file, which
        /// of the extension determines the image codec.
        /// </summary>
        void save(const std::string& path);

        /// <summary>
        /// Creates an unordered access view for the back buffer, which must
        /// have been created before.
        /// </summary>
        virtual winrt::com_ptr<ID3D11UnorderedAccessView> to_uav(void);

        /// <summary>
        /// Enables or disables used of reversed 32-bit depth buffer.
        /// </summary>
        void use_reversed_depth_buffer(const bool isEnabled);

        render_target_base& operator =(const render_target_base&) = delete;

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="device">The device to create the target on.</param>
        render_target_base(const trrojan::device& device = nullptr);

        /// <summary>
        /// Set <paramref name="backBuffer" /> as back buffer and creates the
        /// render target view for it. Furthermore, allocate a depth buffer of
        /// the same size and create the view for it.
        /// </summary>
        /// <remarks>
        /// <see cref="_dsv" /> and <see cref="_rtv" /> must have been deleted
        /// before this method can be called.
        /// </remarks>
        void set_back_buffer(winrt::com_ptr<ID3D11Texture2D> backBuffer);

        /// <summary>
        /// Sets the D3D device and updates the immediate context.
        /// </summary>
        void set_device(winrt::com_ptr<ID3D11Device> device);

        /// <summary>
        /// Clear value for the depth buffer.
        /// </summary>
        float _depth_clear;

        /// <summary>
        /// A depth-stencil state that is being activated with the target.
        /// </summary>
        winrt::com_ptr<ID3D11DepthStencilState> _dss;

        /// <summary>
        /// The depth/stencil view.
        /// </summary>
        winrt::com_ptr<ID3D11DepthStencilView> _dsv;

        /// <summary>
        /// The render target view.
        /// </summary>
        winrt::com_ptr<ID3D11RenderTargetView> _rtv;

    private:

        /// <summary>
        /// The device the render target lives on.
        /// </summary>
        winrt::com_ptr<ID3D11Device> _device;

        /// <summary>
        /// The immediate context of <see cref="_device" />.
        /// </summary>
        winrt::com_ptr<ID3D11DeviceContext> _device_context;

        /// <summary>
        /// A staging texture for saving the back buffer to disk.
        /// </summary>
        winrt::com_ptr<ID3D11Texture2D> _staging_texture;
    };


    /// <summary>
    /// Pointer to a generic render target.
    /// </summary>
    typedef std::shared_ptr<render_target_base> render_target;
}
}
