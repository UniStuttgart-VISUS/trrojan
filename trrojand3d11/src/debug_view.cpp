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
trrojan::d3d11::debug_view::debug_view(void) : hWnd(NULL), isRunning(true) {
    try {
        this->constants = std::make_unique<DebugConstants>();
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
    if (this->hWnd != NULL) {
        ::DestroyWindow(this->hWnd);
        this->hWnd = NULL;
    }

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
    this->rtv = nullptr;

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

        this->set_rtv(backBuffer.p);
    }

    this->viewport.TopLeftX = 0.0f;
    this->viewport.TopLeftY = 0.0f;
    this->viewport.Width = static_cast<float>(width);
    this->viewport.Height = static_cast<float>(height);
    this->viewport.MinDepth = 0.0f;
    this->viewport.MaxDepth = 1.0f;

    this->constants->ViewportSize.x = this->viewport.Width;
    this->constants->ViewportSize.y = this->viewport.Height;
    this->cbConstants = create_buffer(this->device, D3D11_USAGE_DEFAULT,
        D3D11_BIND_CONSTANT_BUFFER, &this->constants, sizeof(DebugConstants));
}


/*
 * trrojan::d3d11::debug_view::show
 */
void trrojan::d3d11::debug_view::show(debugable& content) {
    auto hResource = content.get_debug_staging_texture();
    this->contentLock = nullptr;
    this->contentView = nullptr;

    if (hResource != nullptr) {
        auto hr = S_OK;
        ATL::CComPtr<ID3D11Texture2D> tex;

        if (SUCCEEDED(hr)) {
            // Open the staging texture shared by 'content'.
            hr = this->device->OpenSharedResource(hResource,
                IID_ID3D11Texture2D, reinterpret_cast<void **>(&tex));
        }

        if (SUCCEEDED(hr)) {
            // Get the lock of the staging texture.
            hr = tex->QueryInterface(&this->contentLock);
        }

        if (SUCCEEDED(hr)) {
            // Create a resource view for it.
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
            D3D11_TEXTURE2D_DESC texDesc;

            tex->GetDesc(&texDesc);
            ::ZeroMemory(&srvDesc, sizeof(srvDesc));
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = texDesc.MipLevels;

            this->constants->ImageSize.x = static_cast<float>(texDesc.Width);
            this->constants->ImageSize.y = static_cast<float>(texDesc.Height);

            hr = this->device->CreateShaderResourceView(tex, &srvDesc,
                &this->contentView);
        }

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
    assert(this->rtv == nullptr);
    assert(this->device == nullptr);
    assert(this->context == nullptr);
    assert(this->hWnd == NULL);

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
            &swapDesc, &this->swapChain, &this->device, nullptr,
            &this->context);
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

        this->set_rtv(backBuffer.p);
    }

    {
        D3D11_RASTERIZER_DESC rasterDesc;
        ::ZeroMemory(&rasterDesc, sizeof(rasterDesc));

        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.CullMode = D3D11_CULL_NONE;
        rasterDesc.DepthClipEnable = FALSE;

        auto hr = device->CreateRasterizerState(&rasterDesc,
            &this->rasteriserState);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        D3D11_SAMPLER_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));

        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

        auto hr = device->CreateSamplerState(&desc, &this->samplerState);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        D3D11_DEPTH_STENCIL_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));

        desc.DepthEnable = FALSE;
        desc.DepthFunc = D3D11_COMPARISON_ALWAYS;

        auto hr = device->CreateDepthStencilState(&desc, &this->depthState);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
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
    static const float CLEAR_COLOUR[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    MSG msg;

    while (this->isRunning) {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                this->isRunning = false;
            }

        } else {
            this->context->ClearRenderTargetView(this->rtv, CLEAR_COLOUR);

            this->context->UpdateSubresource(this->cbConstants, 0, nullptr,
                this->constants.get(), 0, 0);

            this->context->VSSetShader(this->vertexShader, nullptr, 0);
            this->context->VSSetConstantBuffers(0, 1, &this->cbConstants.p);

            this->context->GSSetShader(nullptr, nullptr, 0);

            this->context->PSSetShader(this->pixelShader, nullptr, 0);
            this->context->PSSetSamplers(0, 1, &this->samplerState.p);
            this->context->PSSetShaderResources(0, 1, &this->contentView.p);

            this->context->IASetInputLayout(nullptr);
            this->context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
            this->context->IASetPrimitiveTopology(
                D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

            this->context->RSSetState(this->rasteriserState);
            this->context->RSSetViewports(1, &this->viewport);

            this->context->OMSetDepthStencilState(this->depthState, 0);
            this->context->OMSetRenderTargets(1, &this->rtv.p, nullptr);

            auto l = this->contentLock;
            if (l != nullptr) {
                l->AcquireSync(0, INFINITE);
                this->context->Draw(4, 0);
                l->ReleaseSync(0);
            }

            this->swapChain->Present(0, 0);
        }
    }
}


/*
 * trrojan::d3d11::debug_view::set_rtv
 */
void trrojan::d3d11::debug_view::set_rtv(ID3D11Texture2D *texture) {
    assert(texture != nullptr);
    assert(this->rtv == nullptr);

    auto hr = this->device->CreateRenderTargetView(texture, nullptr,
        &this->rtv);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
}
