// <copyright file="debug_render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/debug_render_target.h"

#include <cassert>
#include <sstream>

#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"

#ifndef _UWP


/*
 * trrojan::d3d12::debug_render_target::debug_render_target
 */
trrojan::d3d12::debug_render_target::debug_render_target(
        const trrojan::device& device) : base(device, 2), _wnd(NULL) {
    assert(this->_wnd.is_lock_free());
    this->_msg_pump = std::thread(std::bind(&debug_render_target::do_msg,
        std::ref(*this)));
}


/*
 * trrojan::d3d12::debug_render_target::~debug_render_target
 */
trrojan::d3d12::debug_render_target::~debug_render_target(void) {
    //auto hWnd = this->hWnd.exchange(NULL);
    //if (hWnd != NULL) {
    //    ::SendMessage(hWnd, WM_CLOSE, 0, 0);
    //}

    if (this->_msg_pump.joinable()) {
        log::instance().write_line(log_level::information, "Please close the "
            "debug view to end the programme.");
        this->_msg_pump.join();
    }
}


/*
 * trrojan::d3d12::debug_render_target::present
 */
UINT trrojan::d3d12::debug_render_target::present(void) {
    assert(this->_swap_chain != nullptr);
    // Swap the buffers.
    this->_swap_chain->Present(0, 0);

    // Switch to the next buffer used by the swap chain.
    auto retval = this->_swap_chain->GetCurrentBackBufferIndex();
    render_target_base::switch_buffer(retval);

    return retval;
}


/*
 * trrojan::d3d12::debug_render_target::resize
 */
void trrojan::d3d12::debug_render_target::resize(const unsigned int width,
        const unsigned int height) {
    log::instance().write_line(log_level::debug, "Resizing debug render target "
        "{:p} to [{}, {}].", static_cast<void *>(this), width, height);

    // Resize the window to match the requested client area. This must be done
    // before the swap chain is created, because the swap chain is created to
    // match the client area of the window.
    {
        DWORD style = ::GetWindowLong(this->_wnd, GWL_STYLE);
        DWORD styleEx = ::GetWindowLong(this->_wnd, GWL_EXSTYLE);
        RECT wndRect;

        wndRect.left = 0;
        wndRect.top = 0;
        wndRect.right = width;
        wndRect.bottom = height;
        if (::AdjustWindowRectEx(&wndRect, style, FALSE, styleEx) == FALSE) {
            auto hr = __HRESULT_FROM_WIN32(::GetLastError());
            throw ATL::CAtlException(hr);
        }

        ::SetWindowPos(this->_wnd, HWND_TOP, 0, 0,
            wndRect.right - wndRect.left,
            wndRect.bottom - wndRect.top,
            SWP_NOMOVE | SWP_SHOWWINDOW);
    }

    if (this->_swap_chain == nullptr) {
        // Initial call to resize, need to create the swap chain.
        assert(this->device() != nullptr);

        while (this->_wnd.load() == NULL) {
            log::instance().write_line(log_level::verbose, "Waiting for the "
                "debug view become available ...");
        }

        this->_swap_chain = this->create_swap_chain(this->_wnd);

        ::ShowWindow(this->_wnd, SW_SHOW);

    } else {
        // Resize an existing swap chain.
        DXGI_SWAP_CHAIN_DESC desc;

        this->reset_buffers();

        {
            auto hr = this->_swap_chain->GetDesc(&desc);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            auto hr = this->_swap_chain->ResizeBuffers(desc.BufferCount,
                width, height, desc.BufferDesc.Format, 0);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

    } /* end if (this->swapChain == nullptr) */

    // Re-create the RTV/DSV.
    {
        std::vector<ATL::CComPtr<ID3D12Resource>> buffers(
            this->pipeline_depth());

        for (UINT i = 0; i < this->pipeline_depth(); ++i) {
            auto hr = this->_swap_chain->GetBuffer(i, ::IID_ID3D12Resource,
                reinterpret_cast<void **>(&buffers[i]));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            std::stringstream name;
            name << "debug_render_target (colour buffer " << i << ")";
            set_debug_object_name(buffers[i].p, name.str().c_str());
        }

        this->set_buffers(std::move(buffers),
            this->_swap_chain->GetCurrentBackBufferIndex());
    }
}


/*
 * trrojan::d3d12::debug_render_target::to_uav
 */
//void trrojan::d3d12::debug_render_target::to_uav(
//        const D3D12_CPU_DESCRIPTOR_HANDLE dst,
//        ID3D12GraphicsCommandList *cmd_list) {
//    if (this->_staging_buffer == nullptr) {
//        ATL::CComPtr<ID3D12Resource> texture;
//
//        {
//            auto hr = this->_swap_chain->GetBuffer(0, ::IID_ID3D12Resource,
//                reinterpret_cast<void **>(&texture));
//            if (FAILED(hr)) {
//                throw ATL::CAtlException(hr);
//            }
//        }
//
//        auto desc = texture->GetDesc();
//        this->_staging_buffer = create_resource(this->device(), desc);
//    }
//
//    this->device()->CreateUnorderedAccessView(this->_staging_buffer, nullptr,
//        nullptr, dst);
//}


/*
 * trrojan::d3d12::debug_render_target::reset_buffers
 */
void trrojan::d3d12::debug_render_target::reset_buffers(void) {
    render_target_base::reset_buffers();
    // Make sure that the staging buffer is re-created when the target UAV
    // is requested the next time.
    this->_staging_buffer = nullptr;
}


/*
 * trrojan::d3d12::debug_render_target::WINDOW_CLASS
 */
const TCHAR *trrojan::d3d12::debug_render_target::WINDOW_CLASS
    = _T("trrojectd3d12dbg");


/*
 * trrojan::d3d12::debug_render_target::wnd_proc
 */
LRESULT trrojan::d3d12::debug_render_target::wnd_proc(HWND hWnd, UINT msg,
        WPARAM wParam, LPARAM lParam) {
    auto that = reinterpret_cast<debug_render_target *>(::GetWindowLongPtrW(
        hWnd, GWLP_USERDATA));
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
 * trrojan::d3d12::debug_render_target::do_msg
 */
void trrojan::d3d12::debug_render_target::do_msg(void) {
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
            wndClass.lpfnWndProc = debug_render_target::wnd_proc;
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
        this->_wnd = ::CreateWindowEx(styleEx,
            WINDOW_CLASS,
            _T("TRRojan"),
            style,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL,
            hInstance,
            this);
        if (this->_wnd.load() == NULL) {
            auto hr = __HRESULT_FROM_WIN32(::GetLastError());
            throw ATL::CAtlException(hr);
        }
    }

    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}


#endif //_UWP