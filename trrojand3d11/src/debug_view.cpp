/// <copyright file="debug_view.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/debug_view.h"

#include <cassert>
#include <sstream>

#include "trrojan/log.h"

#include "trrojan/d3d11/utilities.h"

#include "DebugPipeline.hlsli"
#include "DebugPixelShader.h"
#include "DebugVertexShader.h"


/*
 * trrojan::d3d11::debug_view::debug_view
 */
trrojan::d3d11::debug_view::debug_view(void) : render_target_base(nullptr),
        hWnd(NULL), isRunning(true) {
    try {
        this->initialise();
        this->msgPump = std::thread(std::bind(&debug_view::message_loop,
            std::ref(*this)));
    } catch (...) {
        this->~debug_view();
    }
}


/*
 * trrojan::d3d11::debug_view::~debug_view
 */
trrojan::d3d11::debug_view::~debug_view(void) {
    this->isRunning = false;
    if (this->msgPump.joinable()) {
        this->msgPump.join();
    }
}


/*
 * trrojan::d3d11::debug_view::resize
 */
void trrojan::d3d11::debug_view::resize(const unsigned int width,
        const unsigned int height) {
    assert(this->hWnd != NULL);
    DXGI_SWAP_CHAIN_DESC desc;

    // Release existing views.
    this->_rtv = nullptr;
    this->_dsv = nullptr;

    {
        auto hr = this->swapChain->GetDesc(&desc);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->swapChain->ResizeBuffers(desc.BufferCount, width,
            height, desc.BufferDesc.Format, 0);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        ATL::CComPtr<ID3D11Texture2D> backBuffer;

        auto hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
            reinterpret_cast<void **>(&backBuffer));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        this->set_back_buffer(backBuffer.p);
    }
}


/*
 * trrojan::d3d11::debug_view::show
 */
void trrojan::d3d11::debug_view::show(debugable& content) {
    auto hRes = content.get_debug_staging_texture();
    if (hRes != nullptr) {
        this->content = nullptr;
        
        auto hr = this->device()->OpenSharedResource(hRes, IID_ID3D11Texture2D, reinterpret_cast<void **>(&this->content));
        if (SUCCEEDED(hr)) {
            ::ShowWindow(this->hWnd, SW_SHOW);
        }
    }
}


/*
 * trrojan::d3d11::debug_view::WINDOW_CLASS
 */
const TCHAR *trrojan::d3d11::debug_view::WINDOW_CLASS
    = _T("trrojectd3d11dbg");


/*
 * trrojan::d3d11::debug_view::wndProc
 */
LRESULT trrojan::d3d11::debug_view::wndProc(HWND hWnd, UINT msg,
        WPARAM wParam, LPARAM lParam) {
    debug_view *that = reinterpret_cast<debug_view *>(
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


/*
 * trrojan::d3d11::debug_view::init
 */
void trrojan::d3d11::debug_view::initialise(void) {
    assert(this->_rtv == nullptr);
    assert(this->_dsv == nullptr);
    assert(this->device() == nullptr);
    assert(this->device_context() == nullptr);
    assert(this->hWnd == NULL);
    ATL::CComPtr<ID3D11Device> device;

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
            wndClass.lpfnWndProc = debug_view::wndProc;
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
        if (this->hWnd == NULL) {
            auto hr = __HRESULT_FROM_WIN32(::GetLastError());
            throw ATL::CAtlException(hr);
        }
    }

    {
        DXGI_SWAP_CHAIN_DESC swapDesc;
        RECT wndRect;

        ::GetClientRect(this->hWnd, &wndRect);

        ::ZeroMemory(&swapDesc, sizeof(swapDesc));
        swapDesc.BufferCount = 2;
        swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapDesc.BufferDesc.Height = wndRect.bottom - wndRect.top;
        swapDesc.BufferDesc.Width = wndRect.right - wndRect.left;
        swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapDesc.OutputWindow = this->hWnd;
        swapDesc.SampleDesc.Count = 1;
        swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapDesc.Windowed = TRUE;

        auto hr = ::D3D11CreateDeviceAndSwapChain(nullptr,
            D3D_DRIVER_TYPE_HARDWARE, NULL, 0, nullptr, 0, D3D11_SDK_VERSION,
            &swapDesc, &this->swapChain, &device, nullptr, nullptr);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        this->set_device(device.p);
    }

    {
        ATL::CComPtr<ID3D11Texture2D> backBuffer;

        auto hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
            reinterpret_cast<void **>(&backBuffer));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        this->set_back_buffer(backBuffer.p);
    }

    this->pixelShader = create_pixel_shader(device, ::DebugPixelShaderBytes);
    this->vertexShader = create_vertex_shader(device, ::DebugVertexShaderBytes);

#if (defined(DEBUG) || defined(_DEBUG))
    ::ShowWindow(this->hWnd, SW_SHOW);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
}


/*
 * trrojan::d3d11::debug_view::run
 */
void trrojan::d3d11::debug_view::message_loop(void) {
    MSG msg;

    while (this->isRunning) {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                this->isRunning = false;
            }

        } else {
            this->clear();
            this->swapChain->Present(0, 0);
        }
    }
}
