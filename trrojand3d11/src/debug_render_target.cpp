/// <copyright file="debug_render_target.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/debug_render_target.h"

#include <cassert>
#include <sstream>

#include "trrojan/log.h"

#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::debug_render_target::debug_render_target
 */
trrojan::d3d11::debug_render_target::debug_render_target(void) : base(nullptr),
        hWnd(NULL) {
    this->msgPump = std::thread(std::bind(&debug_render_target::doMsg,
        std::ref(*this)));
}


/*
 * trrojan::d3d11::debug_render_target::~debug_render_target
 */
trrojan::d3d11::debug_render_target::~debug_render_target(void) {
    //auto hWnd = this->hWnd.exchange(NULL);
    //if (hWnd != NULL) {
    //    ::SendMessage(hWnd, WM_CLOSE, 0, 0);
    //}

    if (this->msgPump.joinable()) {
        log::instance().write_line(log_level::information, "Please close the "
            "debug view to end the programme.");
        this->msgPump.join();
    }
}


/*
 * trrojan::d3d11::debug_render_target::present
 */
void trrojan::d3d11::debug_render_target::present(void) {
    if (this->swapChain != nullptr) {
        this->swapChain->Present(0, 0);
    }
}


/*
 * trrojan::d3d11::debug_render_target::resize
 */
void trrojan::d3d11::debug_render_target::resize(const unsigned int width,
        const unsigned int height) {
    ATL::CComPtr<ID3D11Texture2D> backBuffer;
    DXGI_SWAP_CHAIN_DESC desc;
    HRESULT hr = S_OK;

    if (this->swapChain == nullptr) {
        /* Initial resize. */
        assert(this->_dsv == nullptr);
        assert(this->_rtv == nullptr);
        assert(this->device() == nullptr);
        assert(this->device_context() == nullptr);

        while (this->hWnd.load() == nullptr) {
            log::instance().write_line(log_level::verbose, "Waiting for the "
                "debug view becoming available ...");
        }

        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferCount = 2;
        desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.BufferDesc.Height = width;
        desc.BufferDesc.Width = height;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.OutputWindow = this->hWnd;
        desc.SampleDesc.Count = 1;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        desc.Windowed = TRUE;

        {
            ATL::CComPtr<ID3D11Device> device;
            UINT deviceFlags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;

#if (defined(DEBUG) || defined(_DEBUG))
            if (supports_debug_layer()) {
                deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

            hr = ::D3D11CreateDeviceAndSwapChain(nullptr,
                D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, nullptr, 0,
                D3D11_SDK_VERSION, &desc, &this->swapChain, &device,
                nullptr, nullptr);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            this->set_device(device);
        }

        ::ShowWindow(this->hWnd, SW_SHOW);

    } else {
        /* Have existing swap chain. */
        assert(this->hWnd.load() != NULL);
        this->_rtv = nullptr;
        this->_dsv = nullptr;

        hr = this->swapChain->GetDesc(&desc);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        hr = this->swapChain->ResizeBuffers(desc.BufferCount, width,
            height, desc.BufferDesc.Format, 0);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

    } /* end if (this->swapChain == nullptr) */
    assert(this->_dsv == nullptr);
    assert(this->_rtv == nullptr);

    /* Resize the window to match the requested client area. */
    {
        DWORD style = ::GetWindowLong(this->hWnd, GWL_STYLE);
        DWORD styleEx = ::GetWindowLong(this->hWnd, GWL_EXSTYLE);
        RECT wndRect;

        wndRect.left = 0;
        wndRect.top = 0;
        wndRect.right = width;
        wndRect.bottom = height;
        if (::AdjustWindowRectEx(&wndRect, style, FALSE, styleEx) == FALSE) {
            auto hr = __HRESULT_FROM_WIN32(::GetLastError());
            throw ATL::CAtlException(hr);
        }

        ::SetWindowPos(this->hWnd, HWND_TOP, 0, 0,
            wndRect.right - wndRect.left,
            wndRect.bottom - wndRect.top,
            SWP_NOMOVE | SWP_SHOWWINDOW);
    }

    /* Re-create the RTV/DSV. */
    hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
        reinterpret_cast<void **>(&backBuffer));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    set_debug_object_name(backBuffer.p, "debug_render_target (colour buffer)");

    this->set_back_buffer(backBuffer.p);
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

        case WM_CLOSE:
            ::PostQuitMessage(0);
            break;

        case WM_KEYDOWN:
            if ((wParam == VK_ESCAPE) && (that != nullptr)) {
                ::PostQuitMessage(0);
            }
            break;

        default:
            retval = ::DefWindowProc(hWnd, msg, wParam, lParam);
            break;
    }

    return retval;
}



/*
 * trrojan::d3d11::debug_render_target::doMsg
 */
void trrojan::d3d11::debug_render_target::doMsg(void) {
    MSG msg;

    const auto hInstance = ::GetModuleHandle(NULL);
    if (hInstance == NULL) {
        auto hr = __HRESULT_FROM_WIN32(::GetLastError());
        throw ATL::CAtlException(hr);
    }

    {
        WNDCLASSEX wndClass;
        if (!::GetClassInfoEx(hInstance, WINDOW_CLASS, &wndClass)) {
            ::ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
            wndClass.cbSize = sizeof(WNDCLASSEX);
            wndClass.style = CS_CLASSDC;
            wndClass.lpfnWndProc = debug_render_target::wndProc;
            wndClass.hInstance = hInstance;
            wndClass.lpszClassName = WINDOW_CLASS;

            if (!::RegisterClassEx(&wndClass)) {
                auto hr = __HRESULT_FROM_WIN32(::GetLastError());
                throw ATL::CAtlException(hr);
            }
        }
    }

    {
        DWORD style = WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX;
        DWORD styleEx = 0;

        /* Create the window. */
        this->hWnd = ::CreateWindowEx(styleEx,
            WINDOW_CLASS,
            _T("TRRojan"),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL,
            hInstance,
            this);
        if (this->hWnd.load() == NULL) {
            auto hr = __HRESULT_FROM_WIN32(::GetLastError());
            throw ATL::CAtlException(hr);
        }
    }

    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}