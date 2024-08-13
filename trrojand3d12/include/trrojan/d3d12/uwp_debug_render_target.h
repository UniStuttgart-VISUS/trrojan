﻿// <copyright file="uwp_debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>
// <author>Christoph Müller</author>

#pragma once

#include <d2d1_3.h>
#include <dwrite_3.h>
#include <wincodec.h>

#include <Windows.h>

#include "trrojan/uwp_render_target_base.h"

#if defined(CREATE_D2D_OVERLAY)
#include "trrojan/d3d12/d2d_overlay.h"
#endif // defined(CREATE_D2D_OVERLAY)

#include "trrojan/d3d12/render_target.h"


namespace trrojan {
namespace d3d12 {

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// The debug view is a render target using a visible window.
    /// </summary>
    /// <remarks>
    /// This debug render target works by copying the data from a UAV that is
    /// used as the actual render target. This way, it is ensured that we
    /// actually see what the real thing is doing rather than having a
    /// completely different setup that might hide or induce undesired effects.
    /// </remarks>
    class TRROJAND3D12_API uwp_debug_render_target
            : public uwp_render_target_base<render_target_base> {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        uwp_debug_render_target(const trrojan::device& device);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~uwp_debug_render_target(void);

        /// <inheritdoc />
        UINT present(const unsigned int sync_interval) override;

        // <inheritdoc />
        void resize(const unsigned int width,
            const unsigned int height) override;

        ///// <inheritdoc />
        //void to_uav(const D3D12_CPU_DESCRIPTOR_HANDLE dst,
        //    ID3D12GraphicsCommandList *cmd_list) override;

    protected:

        /// <inheritdoc />
        void reset_buffers(void) override;

    private:

        typedef uwp_render_target_base<render_target_base> base;

        /// <summary>
        /// The staging buffer used to back the UAV that we provide to external
        /// users for writing to the debug render target. If the render target
        /// is enabled, this staging buffer will actually become the target.
        /// Before presenting the debug target, its content will be copied to
        /// the back buffer of the swap chain.
        /// </summary>
        winrt::com_ptr<ID3D12Resource> staging_buffer_;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        winrt::com_ptr<IDXGISwapChain3> swap_chain_;

        /// Direct2D drawing components
#if defined(CREATE_D2D_OVERLAY)
        std::unique_ptr<d2d_overlay> d2d_overlay_;
#endif // defined(CREATE_D2D_OVERLAY)
    };
#endif /* defined(TRROJAN_FOR_UWP) */

} /* namespace d3d12 */
} /* namespace trrojan */
