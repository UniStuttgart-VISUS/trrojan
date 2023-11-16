// <copyright file="uwp_debug_render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Michael Becher</author>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/uwp_debug_render_target.h"

#include <d3d11on12.h>

#include <cassert>
#include <sstream>

#include <winrt/windows.foundation.h>
#include <winrt/windows.graphics.display.h>
#include <winrt/windows.ui.core.h>

#include "trrojan/log.h"

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/utilities.h"


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d12::uwp_debug_render_target::uwp_debug_render_target
 */
trrojan::d3d12::uwp_debug_render_target::uwp_debug_render_target(
    const trrojan::device& device) : base(device, 2){ }


/*
 * trrojan::d3d12::uwp_debug_render_target::~uwp_debug_render_target
 */
trrojan::d3d12::uwp_debug_render_target::~uwp_debug_render_target(void) {
    this->clear_state();
}


/*
 * trrojan::d3d12::uwp_debug_render_target::present
 */
UINT trrojan::d3d12::uwp_debug_render_target::present(void) {
    assert(this->swap_chain_ != nullptr);

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
        std::wstring w_text = std::wstring(&wstr[0], &wstr[0] + wchars_num);

        this->d2d_overlay_->begin_draw(this->buffer_index());
        //this->d2d_overlay_->begin_draw(this->swap_chain_->GetCurrentBackBufferIndex());
        this->d2d_overlay_->draw_text(w_text.c_str(), L"Segoue UI", 14.f, D2D1::ColorF::White);
        this->d2d_overlay_->end_draw();
    }
#endif // defined(CREATE_D2D_OVERLAY)

    //window_.get().Dispatcher().ProcessEvents(winrt::Windows::UI::Core::CoreProcessEventsOption::ProcessAllIfPresent);

    // Swap the buffers.
    this->swap_chain_->Present(1, 0);

    // Switch to the next buffer used by the swap chain.
    auto retval = this->swap_chain_->GetCurrentBackBufferIndex();
    render_target_base::switch_buffer(retval);
    
    return retval;
}


/*
 * trrojan::d3d12::uwp_debug_render_target::resize
 */
void trrojan::d3d12::uwp_debug_render_target::resize(const unsigned int width,
    const unsigned int height) {

    this->try_resize_view(width, height);

    if (this->swap_chain_ == nullptr) {
        // Initial call to resize, need to create the swap chain.
        assert(this->device() != nullptr);

        this->clear_state();
        this->swap_chain_ = this->create_swap_chain(width, height,
            this->_window.get());
    }
    else {
        // Resize an existing swap chain.
        this->reset_buffers();

        DXGI_SWAP_CHAIN_DESC1 desc;
        {
            auto hr = this->swap_chain_->GetDesc1(&desc);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

#if defined(CREATE_D2D_OVERLAY)
        this->d2d_overlay_->on_resize();

        // killing the pointer and completely rebuilding the d2d_overlay also works
        //this->d2d_overlay_.reset(); 
#endif // defined(CREATE_D2D_OVERLAY)

        {
            auto hr = this->swap_chain_->ResizeBuffers(
                desc.BufferCount,
                width,
                height,
                desc.Format,
                0
            );
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    } /* end if (this->swap_chain_ == nullptr) */

    // Re-create the RTV/DSV.
    {
        std::vector<ATL::CComPtr<ID3D12Resource>> buffers(
            this->pipeline_depth());

        for (UINT i = 0; i < this->pipeline_depth(); ++i) {
            auto hr = this->swap_chain_->GetBuffer(i, ::IID_ID3D12Resource,
                reinterpret_cast<void**>(&buffers[i]));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            std::stringstream name;
            name << "uwp_debug_render_target (colour buffer " << i << ")";
            set_debug_object_name(buffers[i].p, name.str().c_str());
        }

        this->set_buffers(std::move(buffers),
            this->swap_chain_->GetCurrentBackBufferIndex());
    }

    // create the necessary objects for d2d_overlay
#if defined(CREATE_D2D_OVERLAY)
    if (this->d2d_overlay_ == nullptr) {
        this->d2d_overlay_ = 
            std::make_unique<d2d_overlay>(
                this->device().p,
                this->command_queue().p,
                this->swap_chain_.p,
                this->pipeline_depth()
            );
    }
    else {
        this->d2d_overlay_->on_resized();
    }
#endif // defined(CREATE_D2D_OVERLAY)
}


/*
 * trrojan::d3d12::uwp_debug_render_target::to_uav
 */
 //void trrojan::d3d12::uwp_debug_render_target::to_uav(
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
  * trrojan::d3d12::uwp_debug_render_target::reset_buffers
  */
void trrojan::d3d12::uwp_debug_render_target::reset_buffers(void) {
    render_target_base::reset_buffers();
    // Make sure that the staging buffer is re-created when the target UAV
    // is requested the next time.
    this->staging_buffer_ = nullptr;
}
#endif /* defined(TRROJAN_FOR_UWP) */
