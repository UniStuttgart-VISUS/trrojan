﻿// <copyright file="benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/benchmark_base.h"

#include <chrono>
#include <ctime>

#include "trrojan/com_error_category.h"
#include "trrojan/executive.h"
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
_D3D_BENCH_DEFINE_FACTOR(sync_interval);

#undef _D3D_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::benchmark_base::~benchmark_base
 */
trrojan::d3d12::benchmark_base::~benchmark_base(void) { }


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
    auto power_collector = initialise_power_collector(c);
#if defined(TRROJAN_FOR_UWP)
    auto window = c.get<executive::window_type>(executive::factor_core_window);
    if (!window) {
        throw std::invalid_argument("A confguration without a core window was "
            "passed to a benchmark running on the UWP.");
    }
#endif /* defined(TRROJAN_FOR_UWP) */

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
            this->_debug_target = nullptr;
        }

        if (this->_debug_target == nullptr) {
#if defined(TRROJAN_FOR_UWP)
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D12 debug render target on {}.", device->name());
            auto uwp_debug_target = std::make_shared<uwp_debug_render_target>(device);
            uwp_debug_target->set_window(window);
            this->_debug_target = uwp_debug_target;
            changed.push_back(factor_viewport); // Force resize of target.
#else /* defined(TRROJAN_FOR_UWP) */
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D12 debug render target.");
            this->_debug_target = std::make_shared<debug_render_target>(device);
            this->_debug_target->resize(1, 1);   // Force resource allocation.
#endif /* defined(TRROJAN_FOR_UWP) */
        }

        // Overwrite device and render target
        this->_render_target = this->_debug_target;
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
            this->_render_target = std::make_shared<bench_render_target>(device);
            //this->render_target->use_reversed_depth_buffer(true);
            this->on_device_switch(*device);
            // If the device has changed, force the viewport to be re-created:
            changed.push_back(factor_viewport);
        }
    } /* end if (isDebugView) */

    // Resize the render target if the viewport has changed.
    if (contains(changed, factor_viewport)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        log::instance().write_line(log_level::verbose, "Resizing the "
            "benchmarking render target to {0} x {1} px ...", vp[0], vp[1]);
        this->_render_target->resize(vp[0], vp[1]);
    }

    // Run the bechmark.
    auto retval = this->on_run(*device, c, power_collector, changed);

    // Save the resulting image if reFsted.
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
    log::instance().write_line(log_level::debug, "Appending {0} command "
        "allocator(s) of type {1} to {2} existing one(s).", cnt,
        static_cast<std::underlying_type<D3D12_COMMAND_LIST_TYPE>::type>(type),
        dst.size());

    dst.reserve(dst.size() + cnt);
    for (UINT i = 0; i < cnt; ++i) {
        dst.push_back(nullptr);
        auto hr = device->CreateCommandAllocator(type,
            IID_ID3D12CommandAllocator,
            reinterpret_cast<void **>(&dst.back()));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
        set_debug_object_name(dst.back(), "benchmark_base command allocator "
            "#{}", dst.size() - 1);
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_command_list
 */
winrt::com_ptr<ID3D12CommandList>
trrojan::d3d12::benchmark_base::create_command_list(
        const command_allocator_list& allocators,
        const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
        ID3D12PipelineState *initial_state) {
    if (frame >= allocators.size()) {
        log::instance().write_line(log_level::error, "The given list of "
            "command allocators only supports {0} frames, but frame #{1} "
            "was requested.", allocators.size(), frame);
        throw std::system_error(E_INVALIDARG, com_category());
    }
    if (allocators[frame] == nullptr) {
        log::instance().write_line(log_level::error, "The command allocator "
            "at position {0} is invalid.", frame);
        throw std::system_error(E_INVALIDARG, com_category());
    }

    winrt::com_ptr<ID3D12CommandList> retval;
    {
        auto device = get_device(allocators[frame].get());
        auto hr = device->CreateCommandList(0, type, allocators[frame].get(),
            initial_state, ::IID_ID3D12CommandList,
            reinterpret_cast<void **>(&retval));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    return retval;
}

/*
 * trrojan::d3d12::benchmark_base::resolve_shader_path
 */
std::string trrojan::d3d12::benchmark_base::resolve_shader_path(
        const std::string& file_name) {
#if defined(TRROJAN_FOR_UWP)
    return plugin::get_directory() + directory_separator_char + "trrojand3d12"
        + directory_separator_char + file_name;
#else /* defined(TRROJAN_FOR_UWP) */
    return plugin::get_directory() + directory_separator_char + file_name;
#endif /* defined(TRROJAN_FOR_UWP) */
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
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_sync_interval, static_cast<unsigned int>(0)));

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
winrt::com_ptr<ID3D12GraphicsCommandList>
trrojan::d3d12::benchmark_base::create_command_bundle(
        const std::size_t allocator, ID3D12PipelineState *initial_state) {
    auto bundle = this->create_command_list(D3D12_COMMAND_LIST_TYPE_BUNDLE,
        allocator, initial_state);

    winrt::com_ptr<ID3D12GraphicsCommandList> retval;
    if (!bundle.try_as(retval)) {
        throw std::system_error(E_NOINTERFACE, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::create_command_list
 */
winrt::com_ptr<ID3D12CommandList>
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
            throw std::system_error(E_INVALIDARG, com_category());
    }
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
    const auto pipeline_depth = this->pipeline_depth();
    this->_descriptor_heaps.reserve(pipeline_depth);
    log::instance().write_line(log_level::debug, "Allocating generic descriptor "
        "heap(s) with {0} entries for {1} frame(s) ...", cnt, pipeline_depth);

    for (UINT f = 0; f < pipeline_depth; ++f) {
        winrt::com_ptr<ID3D12DescriptorHeap> heap;

        auto hr = device->CreateDescriptorHeap(&desc,
            ::IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&heap));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }

        set_debug_object_name(heap, "CBV/SRV/UAV descriptor heap for "
            "frame #{0}", f);
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
    log_msg += std::to_string(descs.size());
    log_msg += " descriptor heap(s) for ";
    log_msg += std::to_string(this->pipeline_depth());
    log_msg += " frame(s) ...";
    log::instance().write_line(log_level::verbose, log_msg);

    for (UINT b = 0; b < this->pipeline_depth(); ++b) {
        for (auto& d : descs) {
            winrt::com_ptr<ID3D12DescriptorHeap> heap;

            auto hr = device->CreateDescriptorHeap(&d,
                ::IID_ID3D12DescriptorHeap,
                reinterpret_cast<void **>(&heap));
            if (FAILED(hr)) {
                throw std::system_error(hr, com_category());
            }

            set_debug_object_name(heap, "Type {:x} descriptor heap for frame "
                "#{}", static_cast<std::underlying_type<
                decltype(d.Type)>::type>(d.Type), b);
            this->_descriptor_heaps.push_back(std::move(heap));
        }
    }
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
trrojan::d3d12::benchmark_base::graphics_command_list
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        const D3D12_COMMAND_LIST_TYPE type, const std::size_t frame,
        ID3D12PipelineState *initial_state) {
    auto cmd_list = this->create_command_list(type, frame, initial_state);

    winrt::com_ptr<ID3D12GraphicsCommandList> retval;
    if (!cmd_list.try_as(retval)) {
        throw std::system_error(E_NOINTERFACE, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
trrojan::d3d12::benchmark_base::graphics_command_list
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        const std::size_t frame, ID3D12PipelineState *initial_state) {
    return this->create_graphics_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT,
        frame, initial_state);
}


/*
 * trrojan::d3d12::benchmark_base::create_graphics_command_list
 */
trrojan::d3d12::benchmark_base::graphics_command_list
trrojan::d3d12::benchmark_base::create_graphics_command_list(
        ID3D12PipelineState *initial_state) {
    assert(!this->_direct_cmd_allocators.empty());
    return this->create_graphics_command_list(D3D12_COMMAND_LIST_TYPE_DIRECT,
        this->buffer_index(), initial_state);
}


/*
 * trrojan::d3d12::benchmark_base::on_device_switch
 */
void trrojan::d3d12::benchmark_base::on_device_switch(device& device) {
    assert(device.d3d_device() != nullptr);

    log::instance().write_line(log_level::verbose, "(Re-) Allocating {0} "
        " descriptor heap(s).", this->_descriptor_heaps.size());
    for (auto& h : this->_descriptor_heaps) {
        if (h != nullptr) {
            auto desc = h->GetDesc();
            h = nullptr;
            auto hr = device.d3d_device()->CreateDescriptorHeap(&desc,
                ::IID_ID3D12DescriptorHeap, reinterpret_cast<void **>(&h));
            if (FAILED(hr)) {
                throw std::system_error(hr, com_category());
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

        log::instance().write_line(log_level::verbose, "(Re-) Allocating {0} "
            " direct command allocator(s).", cnt);
        create_command_allocators(this->_direct_cmd_allocators,
            device.d3d_device().get(), D3D12_COMMAND_LIST_TYPE_DIRECT, cnt);
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
            throw std::system_error(hr, com_category());
        }
    }

    {
        auto hr = cmd_list->Reset(this->_direct_cmd_allocators[frame].get(),
            initial_state);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
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
    if (this->_render_target != nullptr) {
        std::string p;

        if (path != nullptr) {
            p = path;
        } else {
            p = to_string<char>(std::chrono::system_clock::now(), true);
        }

        {
            // If file name does not have a user-defined extension to select
            // the format, use PNG as the default.
            auto ext = trrojan::get_extension(p);
            if (ext.empty()) {
                p += ".png";
            }
        }

        this->_render_target->save(p);
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
winrt::com_ptr<Id3d12UnorderedAccessView>
trrojan::d3d12::benchmark_base::switch_to_uav_target(void) {
    assert(this->render_target != nullptr);
    auto ctx = this->render_target->device_context();
    Id3d12RenderTargetView *rtv[] = { nullptr };
    ctx->OMSetDepthStencilState(nullptr, 0);
    ctx->OMSetRenderTargets(1, rtv, nullptr);
    return this->render_target->to_uav();
}
#endif
