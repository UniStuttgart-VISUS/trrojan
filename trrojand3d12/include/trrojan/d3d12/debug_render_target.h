// <copyright file="debug_render_target.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <atlbase.h>
#include <Windows.h>

#include "trrojan/d3d12/render_target.h"


/* Forward declatations. */
struct DebugConstants;


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// The debug view is a render target using a visible window.
    /// </summary>
    /// <remarks>
    /// This debug render target works by copying the data from a UAV that is
    /// used as the actual render target. This way, it is ensured that we
    /// actually see what the real thing is doing rather than having a
    /// completely different setup that might hide or induce undesired effects.
    /// </remarks>
    class TRROJAND3D12_API debug_render_target : public render_target_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        debug_render_target(const trrojan::device& device);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debug_render_target(void);

        /// <inheritdoc />
        void present(ID3D12GraphicsCommandList *cmd_list) override;

        // <inheritdoc />
        void resize(const unsigned int width,
            const unsigned int height) override;

        ///// <inheritdoc />
        //void to_uav(const D3D12_CPU_DESCRIPTOR_HANDLE dst,
        //    ID3D12GraphicsCommandList *cmd_list) override;

        /// <inheritdoc />
        void wait_for_frame(void) override;

    protected:

        /// <inheritdoc />
        void reset_buffers(void) override;

    private:

        typedef render_target_base base;

        /// <summary>
        /// The name of the window class we are registering for the debug view.
        /// </summary>
        static const TCHAR *WINDOW_CLASS;

        /// <summary>
        /// The handler for window messages which makes the message pump exit if
        /// the escape key was pressed.
        /// </summary>
        static LRESULT WINAPI wnd_proc(HWND hWnd, UINT msg, WPARAM wParam,
            LPARAM lParam);

        /// <summary>
        /// Runs the message dispatcher.
        /// </summary>
        void do_msg(void);

        /// <summary>
        /// The message pumping thread.
        /// </summary>
        std::thread _msg_pump;

        /// <summary>
        /// The staging buffer used to back the UAV that we provide to external
        /// users for writing to the debug render target. If the render target
        /// is enabled, this staging buffer will actually become the target.
        /// Before presenting the debug target, its content will be copied to
        /// the back buffer of the swap chain.
        /// </summary>
        ATL::CComPtr<ID3D12Resource> _staging_buffer;

        /// <summary>
        /// The swap chain for the window.
        /// </summary>
        ATL::CComPtr<IDXGISwapChain3> _swap_chain;

        /// <summary>
        /// The handle of the debug window.
        /// </summary>
        std::atomic<HWND> _wnd;
    };
}
}
