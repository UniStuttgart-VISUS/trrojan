// <copyright file="uwp_debug_render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>

#if defined(TRROJAN_FOR_UWP)
#include <unknwn.h>

#include "trrojan/d3d11/uwp_debug_render_target.h"

#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>

#include <cassert>
#include <codecvt>
#include <sstream>

#include "trrojan/log.h"

#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::uwp_debug_render_target::~uwp_debug_render_target
 */
trrojan::d3d11::uwp_debug_render_target::~uwp_debug_render_target(void) {
    // release all things?
    this->_rtv = nullptr;
    this->_dsv = nullptr;
    this->_uav = nullptr;
    if (this->m_d2dContext != nullptr) {
        this->m_d2dContext->SetTarget(nullptr);
    }
    this->m_d2dTargetBitmap = nullptr;
    this->swapChain = nullptr;

    if (this->device_context() != nullptr) {
        this->device_context()->ClearState();
        this->device_context()->Flush();
    }

    // TODO: kill d2d_overlay_ explicitly?
}


/*
 * trrojan::d3d11::uwp_debug_render_target::present
 */
void trrojan::d3d11::uwp_debug_render_target::present(
        const UINT sync_interval) {
    if (this->_uav != nullptr) {
        assert(this->swapChain != nullptr);
        ATL::CComPtr<ID3D11Texture2D> dst;
        ATL::CComPtr<ID3D11Resource> res;
        ATL::CComPtr<ID3D11Texture2D> src;

        {
            auto hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
                reinterpret_cast<void**>(&dst));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            this->_uav->GetResource(&res);
            auto hr = res->QueryInterface(&src);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        this->device_context()->CopyResource(dst, src);
    }

    {
#if defined(CREATE_D2D_OVERLAY)
        if (this->d2d_overlay_) {
            std::string log;
            auto log_entries = log::instance().last(16);
            for (const auto& ls : log_entries) {
                log += ls;
            }
            int wchars_num = MultiByteToWideChar(CP_UTF8, 0, log.c_str(), -1, NULL, 0);
            wchar_t* wstr = new wchar_t[wchars_num];
            MultiByteToWideChar(CP_UTF8, 0, log.c_str(), -1, wstr, wchars_num);
            m_text = std::wstring(&wstr[0], &wstr[0] + wchars_num);

            this->d2d_overlay_->begin_draw();
            this->d2d_overlay_->draw_text(m_text.c_str(), L"Segoue UI", 14.f, D2D1::ColorF::White);
            this->d2d_overlay_->end_draw();
        }
#endif // defined(CREATE_D2D_OVERLAY)
    }

    //m_window.get().Dispatcher().ProcessEvents(winrt::Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);

    if (this->swapChain != nullptr) {
        this->swapChain->Present(sync_interval, 0);
    }

}


/*
 * trrojan::d3d11::uwp_debug_render_target::resize
 */
void trrojan::d3d11::uwp_debug_render_target::resize(const unsigned int width,
        const unsigned int height) {
    ATL::CComPtr<ID3D11Texture2D> backBuffer;
    DXGI_SWAP_CHAIN_DESC1 desc;
    HRESULT hr = S_OK;

    if (this->swapChain == nullptr) {
        /* Initial resize. */
        assert(this->_dsv == nullptr);
        assert(this->_rtv == nullptr);
        assert(this->_uav == nullptr);
        assert(this->device() == nullptr);
        assert(this->device_context() == nullptr);

        ::ZeroMemory(&desc, sizeof(desc));
        // desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.Height = width;
        desc.Width = height;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // | DXGI_USAGE_UNORDERED_ACCESS;
        desc.BufferCount = 2; // Use double-buffering to minimize latency. Also required by swap effect.
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
        desc.Flags = 0;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        //desc.Windowed = TRUE;

        {
            ATL::CComPtr<ID3D11Device> device;
            UINT deviceFlags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if (defined(DEBUG) || defined(_DEBUG))
            if (supports_debug_layer()) {
                deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
            }
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

            hr = D3D11CreateDevice(
                nullptr,                    // Specify nullptr to use the default adapter.
                D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
                0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
                deviceFlags,                
                nullptr,                    // List of feature levels this app can support.
                0,                          // Size of the list above.
                D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Runtime apps.
                &device,                    // Returns the Direct3D device created.
                nullptr,                    // Returns feature level of device created.
                nullptr                     // Returns the device immediate context.
            );

            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            // This sequence obtains the DXGI factory that was used to create the Direct3D device above.
            winrt::com_ptr<IDXGIDevice3> dxgiDevice;
            hr = device->QueryInterface(__uuidof(IDXGIDevice3), reinterpret_cast<void**>(&dxgiDevice));
            if (FAILED(hr) || !dxgiDevice) {
                throw ATL::CAtlException(hr);
            }

            winrt::com_ptr<IDXGIAdapter> dxgiAdapter;
            winrt::check_hresult(
                dxgiDevice->GetAdapter(dxgiAdapter.put())
            );

            winrt::com_ptr<IDXGIFactory2> dxgiFactory;
            winrt::check_hresult(
                dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
            );

            winrt::check_hresult(
                dxgiFactory->CreateSwapChainForCoreWindow(
                    device,
                    winrt::get_unknown(_window.get()),
                    &desc,
                    nullptr,
                    &this->swapChain
                )
            );

            this->set_device(device);

            // Initialize Direct2D resources
            {
                // Initialize the Direct2D Factory.
                D2D1_FACTORY_OPTIONS options;
                ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

                winrt::check_hresult(
                    D2D1CreateFactory(
                        D2D1_FACTORY_TYPE_SINGLE_THREADED,
                        __uuidof(ID2D1Factory3),
                        &options,
                        m_d2dFactory.put_void()
                    )
                );

                // Initialize the DirectWrite Factory.
                winrt::check_hresult(
                    DWriteCreateFactory(
                        DWRITE_FACTORY_TYPE_SHARED,
                        __uuidof(IDWriteFactory3),
                        reinterpret_cast<IUnknown**>(m_dwriteFactory.put())
                    )
                );

                // Initialize the Windows Imaging Component (WIC) Factory.
                winrt::check_hresult(
                    CoCreateInstance(
                        CLSID_WICImagingFactory2,
                        nullptr,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&m_wicFactory)
                    )
                );

                // Create the Direct2D device object and a corresponding context.
                //winrt::com_ptr<IDXGIDevice3> dxgiDevice;
                //hr = _device->QueryInterface(__uuidof(IDXGIDevice3), reinterpret_cast<void**>(&dxgiDevice));
                //if (FAILED(hr) || !dxgiDevice) {
                //    throw ATL::CAtlException(hr);
                //}

                winrt::check_hresult(
                    m_d2dFactory->CreateDevice(
                        dxgiDevice.get(), 
                        m_d2dDevice.put()
                    )
                );
                
                winrt::check_hresult(
                    m_d2dDevice->CreateDeviceContext(
                        D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
                        m_d2dContext.put()
                    )
                );

                winrt::com_ptr<IDWriteTextFormat> textFormat;
                winrt::check_hresult(
                        m_dwriteFactory->CreateTextFormat(
                        L"Segoe UI",
                        nullptr,
                        DWRITE_FONT_WEIGHT_LIGHT,
                        DWRITE_FONT_STYLE_NORMAL,
                        DWRITE_FONT_STRETCH_NORMAL,
                        12.0f,
                        L"en-US",
                        textFormat.put()
                    )
                );

                //winrt::check_hresult(
                textFormat.as(m_textFormat);
                //);

                winrt::check_hresult(
                    m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
                );

                winrt::check_hresult(
                    m_d2dFactory->CreateDrawingStateBlock(m_stateBlock.put())
                );

                winrt::check_hresult(
                    m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), m_whiteBrush.put())
                );
            }
        }

#if defined(CREATE_D2D_OVERLAY)
        if (this->swapChain != nullptr) {
            winrt::com_ptr<IDXGISwapChain> sc;
            auto hr = this->swapChain->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&sc));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            winrt::com_ptr<ID3D11Device> dev;
            hr = this->swapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&dev));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            this->d2d_overlay_ = std::make_unique<d2d_overlay>(dev.get(), sc.get());
        }
#endif // defined(CREATE_D2D_OVERLAY)

    } else {
        /* Have existing swap chain. */
        this->_rtv = nullptr;
        this->_dsv = nullptr;
        this->_uav = nullptr;
        this->m_d2dContext->SetTarget(nullptr);
        this->m_d2dTargetBitmap = nullptr;

        hr = this->swapChain->GetDesc1(&desc);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

#if defined(CREATE_D2D_OVERLAY)
        this->d2d_overlay_->on_resize();
#endif // defined(CREATE_D2D_OVERLAY)

        hr = this->swapChain->ResizeBuffers(desc.BufferCount, width,
            height, desc.Format, 0);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

#if defined(CREATE_D2D_OVERLAY)
        this->d2d_overlay_->on_resized();
#endif // defined(CREATE_D2D_OVERLAY)

    } /* end if (this->swapChain == nullptr) */
    assert(this->_dsv == nullptr);
    assert(this->_rtv == nullptr);

    // Try to resize the window to match the requested client area. This might
    // not always work, for example on the Xbox, but the back buffer should
    // still have the requested size.
    this->try_resize_view(width, height);

    /* Re-create the RTV/DSV. */
    //hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
    //    reinterpret_cast<void **>(&backBuffer));
    //if (FAILED(hr)) {
    //    throw ATL::CAtlException(hr);
    //}
    //winrt::com_ptr<ID3D11Texture2D> bb 
    //    = winrt::capture<ID3D11Texture2D>(this->swapChain, &IDXGISwapChain1::GetBuffer, 0);

    // Draw to texture and reference texture in uav to copy it to swap chain each frame
    D3D11_TEXTURE2D_DESC texDesc;
    
    // TODO: could optimise this to prevent unnecessary re-creates.
    ::ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.ArraySize = 1;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    // UAV does not support BGRA: texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    //texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.Width = width;
    
    hr = this->device()->CreateTexture2D(&texDesc, nullptr, &backBuffer);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
    
    this->_uav = create_uav(backBuffer);

    set_debug_object_name(backBuffer.p, "uwp_debug_render_target (colour buffer)");

    this->set_back_buffer(backBuffer.p);


    // 2D rendering stuff
    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            _dpi,
            _dpi
        );

    winrt::com_ptr<IDXGISurface2> dxgiBackBuffer;
    winrt::check_hresult(
        this->swapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer))
    );

    winrt::check_hresult(
        m_d2dContext->CreateBitmapFromDxgiSurface(
            dxgiBackBuffer.get(),
            &bitmapProperties,
            m_d2dTargetBitmap.put()
        )
    );

    m_d2dContext->SetTarget(m_d2dTargetBitmap.get());
    //m_d2dContext->SetDpi(m_effectiveDpi, m_effectiveDpi);

    // Grayscale text anti-aliasing is recommended for all Microsoft Store apps.
    m_d2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}


/*
 * trrojan::d3d11::uwp_debug_render_target::to_uav
 */
ATL::CComPtr<ID3D11UnorderedAccessView>
trrojan::d3d11::uwp_debug_render_target::to_uav(void) {
    if (this->_uav == nullptr) {
        D3D11_TEXTURE2D_DESC desc;
        ATL::CComPtr<ID3D11Texture2D> texture;

        {
            auto hr = this->swapChain->GetBuffer(0, IID_ID3D11Texture2D,
                reinterpret_cast<void **>(&texture));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        texture->GetDesc(&desc);
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        texture = nullptr;

        {
            auto hr = this->device()->CreateTexture2D(&desc, nullptr, &texture);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        this->_uav = create_uav(texture);
    }

    return this->_uav;
}
#endif /* defined(TRROJAN_FOR_UWP) */
