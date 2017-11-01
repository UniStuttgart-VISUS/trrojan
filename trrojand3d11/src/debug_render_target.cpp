/// <copyright file="debug_render_target.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/debug_render_target.h"

#include <cassert>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::debug_render_target::~debug_render_target
 */
trrojan::d3d11::debug_render_target::~debug_render_target(void) {
    this->stop();
}


/*
 * trrojan::d3d11::debug_render_target::resize
 */
void trrojan::d3d11::debug_render_target::resize(const unsigned int width,
        const unsigned int height) {
    if (this->swapChain == nullptr) {
        assert(this->device == nullptr);
        assert(this->deviceContext == nullptr);
        assert(this->hWnd == NULL);

        auto hInstance = ::GetModuleHandle(NULL);
        if (hInstance == NULL) {
            throw std::runtime_error("Failed to retrieve instance handle.");
        }

        WNDCLASSEX wndClass;
        if (!::GetClassInfoEx(hInstance, WINDOW_CLASS, &wndClass)) {
            ::ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
            wndClass.cbSize = sizeof(WNDCLASSEX);
            wndClass.style = CS_CLASSDC;
            wndClass.lpfnWndProc = debug_render_target::wndProc;
            wndClass.hInstance = hInstance;
            wndClass.lpszClassName = WINDOW_CLASS;

            if (!::RegisterClassEx(&wndClass)) {
                throw std::runtime_error("Failed to register window class for "
                    "Direct3D 11 debug render target.");
            }
        }

        DWORD style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
        DWORD styleEx = 0;
        RECT wndRect;

        wndRect.left = 0;
        wndRect.top = 0;
        wndRect.right = width;
        wndRect.bottom = height;
        if (::AdjustWindowRectEx(&wndRect, style, FALSE, styleEx) == FALSE) {
            throw std::runtime_error("Failed to compute window size for "
                "Direct3D 11 debug render target.");
        }

        /* Create the window. */
        this->hWnd = ::CreateWindowEx(styleEx,
            WINDOW_CLASS,
            _T("TRRojan"),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            wndRect.right - wndRect.left, wndRect.bottom - wndRect.top,
            NULL, NULL,
            hInstance,
            this);
        if (this->hWnd == NULL) {
            throw std::runtime_error("Failed to create window for Direct3D 11 "
                "debug render target.");
        }

        auto hr = ::D3D11CreateDeviceAndSwapChain(nullptr,
            D3D_DRIVER_TYPE_HARDWARE, NULL, 0, nullptr, 0, D3D11_SDK_VERSION,
            nullptr, &this->swapChain, &this->device, nullptr,
            &this->deviceContext);
        if (FAILED(hr)) {
            log::instance().write(log_level::error, "Failed to create Direct3D "
                "11 with swap chain (error 0x%x).\n", hr);
            throw std::runtime_error("Failed to create device and swap chain.");
        }

    } else {
        render_target_base::resize(width, height);
    }

}


/*
 * trrojan::d3d11::debug_render_target::run
 */
int trrojan::d3d11::debug_render_target::run(debugable object) {
    if (object != nullptr) {
        MSG msg;

        while (this->isRunning) {
            if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);

            } else {
                this->deviceContext->ClearRenderTargetView(this->rtv, { 0 });
                this->object->draw_debug_view(this->device,
                    this->deviceContext);
                this->swapChain->Present(0, 0);
            }
        }
    } /* end if (object != nullptr) */

    return 0;
}


/*
 * trrojan::d3d11::debug_render_target::start
 */
void trrojan::d3d11::debug_render_target::start(debugable object) {
    bool expected = false;
    if (!this->isRunning.compare_exchange_strong(expected, true)) {
        throw std::logic_error("The debug_render_target cannot be started "
            "while it is running.");
    }

    if (object != nullptr) {
        this->msgPump = std::thread(std::bind(&debug_render_target::run,
            std::ref(*this), object));
    }
}


/*
 * trrojan::d3d11::debug_render_target::stop
 */
void trrojan::d3d11::debug_render_target::stop(void) {
    this->isRunning = false;
    if (this->msgPump.joinable()) {
        this->msgPump.join();
    }
}


/*
 * trrojan::d3d11::debug_render_target::WINDOW_CLASS
 */
const TCHAR *trrojan::d3d11::debug_render_target::WINDOW_CLASS
    = _T("trrojectd3d11dbg");


/*
 * trrojan::d3d11::debug_render_target::wndProc
 */
LRESULT trrojan::d3d11::debug_render_target::wndProc(HWND hWnd, UINT msg,
        WPARAM wParam, LPARAM lParam) {
    debug_render_target *that = reinterpret_cast<debug_render_target *>(
        ::GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    CREATESTRUCTW *cs = nullptr;
    LRESULT retval = 0;

    switch (msg) {
        case WM_CREATE:
            cs = reinterpret_cast<CREATESTRUCTW *>(lParam);
            if ((::SetWindowLongPtrW(hWnd, GWLP_USERDATA,
                    reinterpret_cast<LONG_PTR>(cs->lpCreateParams)) == 0)) {
                retval = ::GetLastError();
            } else {
                retval = 0;
            }
            break;

        case WM_KEYDOWN:
            if ((wParam == VK_ESCAPE) && (that != nullptr)) {
                that->isRunning = false;
            }
            break;

        case WM_QUIT:
            if (that != nullptr) {
                that->isRunning = false;
            }
            break;

        case WM_SIZE:
            if (that != nullptr) {
                that->resize(LOWORD(lParam), HIWORD(lParam));
            }
            break;

        default:
            retval = ::DefWindowProc(hWnd, msg, wParam, lParam);
            break;
    }

    return retval;
}
