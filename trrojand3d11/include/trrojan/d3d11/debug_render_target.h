/// <copyright file="debug_render_target.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include <Windows.h>

#include "trrojan/d3d11/debugable.h"
#include "trrojan/d3d11/render_target_base.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements an on-screen render target for debugging purposes.
    /// </summary>
    class TRROJAND3D11_API debug_render_target : public render_target_base {

    public:

        /// <summary>
        /// Pointer to an object that can be displayed using this target.
        /// </summary>
        typedef std::shared_ptr<trrojan::d3d11::debugable> debugable;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline debug_render_target(void) : render_target_base(nullptr),
            hWnd(NULL), isRunning(false) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~debug_render_target(void);

        /// <summary>
        /// Resize the render target to the given dimension.
        /// </summary>
        /// <remarks>
        /// The render target needs to be resized at least once to allocate
        /// all graphics resources. <see cref="run" /> and <see cref="start" />
        /// must not be called before this method was called the first time.
        /// </remarks>
        /// <param name="width"></param>
        /// <param name="const"></param>
        virtual void resize(const unsigned int width,
            const unsigned int height);

        /// <summary>
        /// Open the debug view and render <paramref name="object" /> until
        /// the escape key was hit.
        /// </summary>
        /// <remarks>
        /// This method will not return until the window was closed. Use
        /// <see cref="start" /> to run the view asynchronously.
        /// </remarks>
        /// <param name="object"></param>
        int run(debugable object);

        /// <summary>
        /// Open the debug view and render <paramref name="object" /> until
        /// the escape key was hit.
        /// </summary>
        /// <param name="object"></param>
        void start(debugable object);

        /// <summary>
        /// If the debug view is running asynchronously, request it to close and
        /// wait for the message pumping thread to exit.
        /// </summary>
        void stop(void);

    private:

        /// <summary>
        /// The name of the window class we are registering for the debug view.
        /// </summary>
        static const TCHAR *WINDOW_CLASS;

        /// <summary>
        /// The handler for window messages which makes the message pump exit if
        /// the escape key was pressed.
        /// </summary>
        static LRESULT WINAPI wndProc(HWND hWnd, UINT msg, WPARAM wParam,
            LPARAM lParam);

        /// <summary>
        /// The handle of the debug window.
        /// </summary>
        HWND hWnd;

        /// <summary>
        /// Determines whether <paramref name="msgPump" /> should run.
        /// </summary>
        std::atomic<bool> isRunning;

        /// <summary>
        /// The message pumping thread.
        /// </summary>
        std::thread msgPump;

        /// <summary>
        /// The object to be shown.
        /// </summary>
        debugable object;
    };
}
}
