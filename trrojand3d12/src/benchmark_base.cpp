// <copyright file="benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/benchmark_base.h"

#include <chrono>
#include <ctime>

#include "trrojan/factor.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/system_factors.h"
#include "trrojan/text.h"

#include "trrojan/d3d12/environment.h"
#include "trrojan/d3d12/bench_render_target.h"
#include "trrojan/d3d12/debug_render_target.h"
#include "trrojan/d3d12/uwp_debug_render_target.h"
#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/utilities.h"


#define _D3D_BENCH_DEFINE_FACTOR(f)                                         \
const std::string trrojan::d3d12::benchmark_base::factor_##f(#f)

_D3D_BENCH_DEFINE_FACTOR(debug_view);
_D3D_BENCH_DEFINE_FACTOR(save_view);

#undef _D3D_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::benchmark_base::~benchmark_base
 */
trrojan::d3d12::benchmark_base::~benchmark_base(void) { }


void trrojan::d3d12::benchmark_base::SetWindow(winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> const& window)
{
    window_ = window;
}


/*
 * trrojan::d3d12::benchmark_base::can_run
 */
bool trrojan::d3d12::benchmark_base::can_run(trrojan::environment env,
        trrojan::device device) const noexcept {
    auto d = std::dynamic_pointer_cast<trrojan::d3d12::device>(device);
    return (d != nullptr);
}


/*
 * trrojan::d3d12::benchmark_base::run
 */
trrojan::result trrojan::d3d12::benchmark_base::run(const configuration& c) {
    std::vector<std::string> changed;
    this->check_changed_factors(c, std::back_inserter(changed));

    auto genericDev = c.get<trrojan::device>(factor_device);
    auto device = std::dynamic_pointer_cast<trrojan::d3d12::device>(genericDev);

    if (device == nullptr) {
        throw std::runtime_error("A configuration without a Direct3D device was "
            "passed to a Direct3D benchmark.");
    }

    // Determine whether we are in debug viewing mode, which will block all
    // device-related factors.
    auto isDebugView = c.get<bool>(factor_debug_view);

    if (contains(changed, factor_debug_view)) {
        // If the debug view has been changed, an implicit switch of the device
        // (from the debug device to the actual device) must be reported.
        changed.push_back(factor_device);
        changed.push_back(factor_viewport);
    }

    if (isDebugView) {
        auto is_device_change = contains(changed, factor_device);
        if (is_device_change) {
            log::instance().write_line(log_level::verbose, "Forcing the "
                "debug render target to be re-created as the device has "
                "changed.");
            this->debug_target_ = nullptr;
        }
        
        if (this->debug_target_ == nullptr) {
#ifdef _UWP
            std::string log_msg = "Lazy creation of d3d12 debug render target on " + device->name();
            log::instance().write_line(log_level::verbose, log_msg);
            auto uwp_debug_target = std::make_shared<uwp_debug_render_target>(device);
            uwp_debug_target->SetWindow(window_);
            this->debug_target_ = uwp_debug_target;
            changed.push_back(factor_viewport); // Force resize of target.
#else // _UWP
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D12 debug render target.");
            this->debug_target = std::make_shared<debug_render_target>();
            this->debug_target->resize(1, 1);   // Force resource allocation.
#endif // _UWP
        }

        // Overwrite device and render target
        this->render_target_ = this->debug_target_;
        //this->render_target->use_reversed_depth_buffer(true);

        // Invoke device switch once the target has been changed.
        if (is_device_change) {
            log::instance().write_line(log_level::verbose, "Reallocating "
                "graphics resources after switch to debug device ...");
            this->on_device_switch(*device);
        }

    } else {
        // Check whether the device has been changed. This should always be done
        // first, because all GPU resources, which depend on the content of the
        // configuration depend on the device as their storage location.
        if (contains_any(changed, factor_device)) {
            log::instance().write_line(log_level::verbose, "The D3D device has "
                "changed. Reallocating all graphics resources ...");
            this->render_target_ = std::make_shared<bench_render_target>(device);
            //this->render_target->use_reversed_depth_buffer(true);
            this->on_device_switch(*device);
            // If the device has changed, force the viewport to be re-created:
            changed.push_back(factor_viewport);
        }
    } /* end if (isDebugView) */

    // Resize the render target if the viewport has changed.
    if (contains(changed, factor_viewport)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        std::string log_msg = "Resizing the benchmarking render target to ";
        log_msg += vp[0];
        log_msg += " x ";
        log_msg += vp[1];
        log_msg += " px ...";
        log::instance().write_line(log_level::verbose, log_msg);
        this->render_target_->resize(vp[0], vp[1]);
    }

    // Run the bechmark.
    auto retval = this->on_run(*device, c, changed);

    // Save the resulting image if requested.
    // TODO: ::save in ::save_target is missing implementation, just throws "TODO"
    if (false) {
        auto ts = c.get<std::string>(system_factors::factor_timestamp);
        std::replace(ts.begin(), ts.end(), ':', '-');
        std::replace(ts.begin(), ts.end(), '.', '-');
        this->save_target(ts.c_str());
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::calc_median
 */
trrojan::d3d12::gpu_timer::millis_type
trrojan::d3d12::benchmark_base::calc_median(
        std::vector<gpu_timer::millis_type>& times) {
    std::sort(times.begin(), times.end());

    auto retval = times[times.size() / 2];

    // If the number of elements is even, the median is defined as the mean of
    // the two middle elements.
    if (times.size() % 2 == 0) {
        retval += times[times.size() / 2 - 1];
        retval *= static_cast<gpu_timer::millis_type>(0.5);
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::create_command_allocators
 */
void trrojan::d3d12::benchmark_base::create_command_allocators(
        command_allocator_list& dst, ID3D12Device *device,
        const D3D12_COMMAND_LIST_TYPE type, const std::size_t cnt) {
    assert(device != nullptr);
    // TODO: calls deprecated forwarding spdlog function
    std::string log_msg = "Appending " + cnt + std::string(" command allocator(s) of type ")
        + std::to_string(type) + " to " + std::to_string(dst.size()) + " existing one(s).";
    log::instance().write_line(log_level::debug, log_msg);

    dst.reserve(dst.size() + cnt);
    for (UINT i = 0; i < cnt; ++i) {
        dst.push_back(nullptr);
        auto hr = device->CreateCommandAllocator(type,
            IID_ID3D12CommandAllocator,
            reinterpret_cast<void **>(&dst.back()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
        set_debug_object_name(dst.back(), "benchmark_base command allocator "
            "#{}", dst.size() - 1);
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_command_list
 */
ATL::CComPtr<ID3D12CommandList>
trrojan::d3d12::benchmark_base::create_command_list(
        const command_allocator_list& allocators,
        const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
        ID3D12PipelineState *initial_state) {
    if (frame >= allocators.size()) {
        std::string log_msg = "The given list of command allocators only supports ";
        log_msg += allocators.size();
        log_msg += " frames, but frame ";
        log_msg += frame;
        log_msg += " was requested.";
        log::instance().write_line(log_level::error, log_msg);
        throw ATL::CAtlException(E_INVALIDARG);
    }
    if (allocators[frame] == nullptr) {
        std::string log_msg = "The command allocator at position ";
        log_msg += frame;
        log_msg = " is invalid.";
        log::instance().write_line(log_level::error, log_msg);
        throw ATL::CAtlException(E_INVALIDARG);
    }

    ATL::CComPtr<ID3D12CommandList> retval;
    {
        auto device = get_device(allocators[frame]);
        auto hr = device->CreateCommandList(0, type, allocators[frame],
            initial_state, ::IID_ID3D12CommandList,
            reinterpret_cast<void **>(&retval));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    return retval;
}

/*
 * trrojan::d3d12::benchmark_base::resolve_shader_path
 */
std::string trrojan::d3d12::benchmark_base::resolve_shader_path(
        const std::string& file_name) {
#if defined(_UWP)
//#error "TODO: retrieve UWP app directory."
    return plugin::get_directory() + directory_separator_char + file_name;
    //return plugin::get_directory
#else /* defined(_UWP) */
    return plugin::get_directory() + directory_separator_char + file_name;
#endif /* defined(_UWP) */
}


/*
 * trrojan::d3d12::benchmark_base::benchmark_base
 */
trrojan::d3d12::benchmark_base::benchmark_base(const std::string& name)
        : trrojan::graphics_benchmark_base(name) {
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_debug_view, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_save_view, false));

    {
        auto dftViewport = std::array<unsigned int, 2> { 1024, 1024 };
        this->_default_configs.add_factor(factor::from_manifestations(
            factor_viewport, dftViewport));
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_buffer_resource_view
 */
void trrojan::d3d12::benchmark_base::create_buffer_resource_view(
        ID3D12Resource *resource, const UINT64 first_element, const UINT cnt,
        const UINT stride, const D3D12_CPU_DESCRIPTOR_HANDLE descriptor) {
    assert(resource != nullptr);
    auto device = get_device(resource);

    D3D12_SHADER_RESOURCE_VIEW_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

    desc.Buffer.FirstElement = first_element;
    desc.Buffer.NumElements = cnt;
    desc.Buffer.StructureByteStride = stride;

    device->CreateShaderResourceView(resource, &desc, descriptor);
}


/*
 * trrojan::d3d12::benchmark_base::create_command_bundle
 */
ATL::CComPtr<ID3D12GraphicsCommandList>
trrojan::d3d12::benchmark_base::create_command_bundle(
        const std::size_t allocator, ID3D12PipelineState *initial_state) {
    auto bundle = this->create_command_list(D3D12_COMMAND_LIST_TYPE_BUNDLE,
        allocator, initial_state);

    ATL::CComPtr<ID3D12GraphicsCommandList> retval;
    auto hr = bundle.QueryInterface(&retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::create_command_list
 */
ATL::CComPtr<ID3D12CommandList>
trrojan::d3d12::benchmark_base::create_command_list(
        const D3D12_COMMAND_LIST_TYPE type,
        const std::size_t frame,
        ID3D12PipelineState *initial_state) {
    switch (type) {
        case D3D12_COMMAND_LIST_TYPE_BUNDLE:
            return create_command_list(this->_bundle_allocators,
                type, frame, initial_state);

        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            return create_command_list(this->_compute_cmd_allocators,
                type, frame, initial_state);

        case D3D12_COMMAND_LIST_TYPE_COPY:
            return create_command_list(this->_copy_cmd_allocators,
                type, frame, initial_state);

        case D3D12_COMMAND_LIST_TYPE_DIRECT:
            return create_command_list(this->_direct_cmd_allocators,
                type, frame, initial_state);

        default:
            std::string log_msg = "Creating a command list of type ";
            log_msg += type;
            log_msg += " is not supported.";
            log::instance().write_line(log_level::error, log_msg);
            throw ATL::CAtlException(E_INVALIDARG);
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
ATL::CComPtr<ID3D12GraphicsCommandList>
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
        ID3D12PipelineState *initial_state) {
    auto cmd_list = this->create_command_list(type, frame, initial_state);

    ATL::CComPtr<ID3D12GraphicsCommandList> retval;
    auto hr = cmd_list.QueryInterface(&retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
ATL::CComPtr<ID3D12GraphicsCommandList>
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        const std::size_t frame, ID3D12PipelineState *initial_state) {
    return this->create_graphics_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT,
        frame, initial_state);
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
ATL::CComPtr<ID3D12GraphicsCommandList>
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        ID3D12PipelineState *initial_state) {
    assert(!this->_direct_cmd_allocators.empty());
    return this->create_graphics_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT,
        this->buffer_index(), initial_state);
}


/*
 * trrojan::d3d12::benchmark_base::create_descriptor_heaps
 */
void trrojan::d3d12::benchmark_base::create_descriptor_heaps(
        ID3D12Device *device, const UINT cnt) {
    assert(device != nullptr);
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.NumDescriptors = cnt;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    this->_descriptor_heaps.clear();
    this->_descriptor_heaps.reserve(this->pipeline_depth());
    std::string log_msg = "Allocating generic descriptor heap(s) with ";
    log_msg += cnt;
    log_msg += " entries for ";
    log_msg += this->pipeline_depth();
    log_msg += " frames(s) ...";
    log::instance().write_line(log_level::debug, log_msg);

    for (UINT f = 0; f < this->pipeline_depth(); ++f) {
        ATL::CComPtr<ID3D12DescriptorHeap> heap;

        auto hr = device->CreateDescriptorHeap(&desc,
            ::IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&heap));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        set_debug_object_name(heap, "CBV/SRV/UAV descriptor heap for frame #{}",
            f);
        this->_descriptor_heaps.push_back(std::move(heap));
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_descriptor_heaps
 */
void trrojan::d3d12::benchmark_base::create_descriptor_heaps(
        ID3D12Device *device,
        const std::vector<D3D12_DESCRIPTOR_HEAP_DESC>& descs) {
    this->_descriptor_heaps.clear();
    this->_descriptor_heaps.reserve(this->pipeline_depth() * descs.size());

    std::string log_msg = "Allocating ";
    log_msg += descs.size();
    log_msg += " descriptor heap(s) for ";
    log_msg += this->pipeline_depth();
    log_msg += " frame(s) ...";
    log::instance().write_line(log_level::verbose, log_msg);

    for (UINT b = 0; b < this->pipeline_depth(); ++b) {
        for (auto& d : descs) {
            ATL::CComPtr<ID3D12DescriptorHeap> heap;

            auto hr = device->CreateDescriptorHeap(&d,
                ::IID_ID3D12DescriptorHeap,
                reinterpret_cast<void **>(&heap));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

            set_debug_object_name(heap, "Type {:x} descriptor heap for frame "
                "#{}", static_cast<std::underlying_type<
                decltype(d.Type)>::type>(d.Type), b);
            this->_descriptor_heaps.push_back(std::move(heap));
        }
    }
}


/*
 * trrojan::d3d12::benchmark_base::on_device_switch
 */
void trrojan::d3d12::benchmark_base::on_device_switch(device& device) {
    assert(device.d3d_device() != nullptr);

    std::string log_msg = "(Re-) Allocating ";
    log_msg += this->_descriptor_heaps.size();
    log_msg += " descriptor heap(s).";
    log::instance().write_line(log_level::verbose, log_msg);
    for (auto& h : this->_descriptor_heaps) {
        if (h != nullptr) {
            auto desc = h->GetDesc();
            h = nullptr;
            auto hr = device.d3d_device()->CreateDescriptorHeap(&desc,
                ::IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&h));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }

    {
        // Note: in contrast to the other kinds of allocators, we force that
        // there is at least one direct allocator for each frame in the
        // pipeline.
        auto cnt = (std::max)(this->_direct_cmd_allocators.size(),
            static_cast<std::size_t>(this->pipeline_depth()));
        this->_direct_cmd_allocators.clear();

        log_msg.clear();
        log_msg = "(Re-) Allocating ";
        log_msg += cnt;
        log_msg += " direct command allocator(s).";
        log::instance().write_line(log_level::verbose, log_msg);
        create_command_allocators(this->_direct_cmd_allocators,
            device.d3d_device(), D3D12_COMMAND_LIST_TYPE_DIRECT, cnt);
    }
}


/*
 * trrojan::d3d12::benchmark_base::reset_command_list
 */
void trrojan::d3d12::benchmark_base::reset_command_list(
        ID3D12GraphicsCommandList *cmd_list, const UINT frame,
        ID3D12PipelineState *initial_state) const {
    assert(cmd_list != nullptr);

    {
        auto hr = this->_direct_cmd_allocators[frame]->Reset();
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = cmd_list->Reset(this->_direct_cmd_allocators[frame],
            initial_state);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}


/*
 * trrojan::d3d12::benchmark_base::reset_command_list
 */
void trrojan::d3d12::benchmark_base::reset_command_list(
        ID3D12GraphicsCommandList *cmd_list,
        ID3D12PipelineState *initial_state) const {
    this->reset_command_list(cmd_list, this->buffer_index(), initial_state);
}


/*
 * trrojan::d3d12::benchmark_base::save_target
 */
void trrojan::d3d12::benchmark_base::save_target(const char *path) {
    if (this->render_target_ != nullptr) {
        std::string p;

        if (path != nullptr) {
            p = path;
        } else {
            p = to_string<char>(std::chrono::system_clock::now(), true);
        }

        {
            // If file name does not have a user-defined extension to select
            // the format, use PNG as the default.
            auto ext = p.find_last_of(extension_separator_char);
            if (ext == std::string::npos) {
                p += ".png";
            }
        }

        this->render_target_->save(p);
    }
}

/*
 * trrojan::d3d12::benchmark_base::set_aspect_from_viewport
 */
void trrojan::d3d12::benchmark_base::set_aspect_from_viewport(
        camera& camera) {
    const auto aspect = this->viewport().Width / this->viewport().Height;
    camera.set_aspect_ratio(aspect);
}


#if TODO
/*
 * trrojan::d3d12::benchmark_base::switch_to_uav_target
 */
ATL::CComPtr<Id3d12UnorderedAccessView>
trrojan::d3d12::benchmark_base::switch_to_uav_target(void) {
    assert(this->render_target != nullptr);
    auto ctx = this->render_target->device_context();
    Id3d12RenderTargetView *rtv[] = { nullptr };
    ctx->OMSetDepthStencilState(nullptr, 0);
    ctx->OMSetRenderTargets(1, rtv, nullptr);
    return this->render_target->to_uav();
}
#endif
