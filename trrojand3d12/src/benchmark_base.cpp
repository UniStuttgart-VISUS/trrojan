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

    // Allocate a command list that is used to perform all transition tasks.
    auto cmdList = device->create_graphics_command_list();

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
        log::instance().write_line(log_level::warning, "Using the debug view "
            "restricts the benchmark to the GPU connected to the display. The "
            "device parameter has no effect.");
        if (this->_debug_target == nullptr) {
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "d3d12 debug render target.");
            this->_debug_target = std::make_shared<debug_render_target>();
            this->_debug_target->resize(1, 1);  // Force resource allocation.
            this->_debug_device = std::make_shared<d3d12::device>(
                this->_debug_target->device());
        }

        // Overwrite device and render target.
        device = this->_debug_device;
        this->_render_target = this->_debug_target;
        //this->render_target->use_reversed_depth_buffer(true);

    } else {
        // Check whether the device has been changed. This should always be done
        // first, because all GPU resources, which depend on the content of the
        // configuration depend on the device as their storage location.
        if (contains_any(changed, factor_device)) {
            log::instance().write_line(log_level::verbose, "The D3D device has "
                "changed. Reallocating all graphics resources ...");
            this->_render_target = std::make_shared<bench_render_target>(device);
            //this->render_target->use_reversed_depth_buffer(true);
            // If the device has changed, force the viewport to be re-created:
            changed.push_back(factor_viewport);
        }
    } /* end if (isDebugView) */

    // Resize the render target if the viewport has changed.
    if (contains(changed, factor_viewport)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        log::instance().write_line(log_level::verbose, "Resizing the "
            "benchmarking render target to %d × %d px ...", vp[0], vp[1]);
        this->_render_target->resize(vp[0], vp[1]);
    }

    this->_render_target->enable(cmdList);
    auto retval = this->on_run(*device, c, changed);

    if (c.get<bool>(factor_save_view)) {
        auto ts = c.get<std::string>(system_factors::factor_timestamp);
        std::replace(ts.begin(), ts.end(), ':', '-');
        std::replace(ts.begin(), ts.end(), '.', '-');
        this->save_target(ts.c_str());
    }

    return retval;
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
 * trrojan::d3d12::benchmark_base::save_target
 */
void trrojan::d3d12::benchmark_base::save_target(const char *path) {
    if (this->_render_target != nullptr) {
        std::string p;

        if (path != nullptr) {
            p = path;
        } else {
            p = to_string<char>(std::chrono::system_clock::now());
        }

        {
            auto ext = p.find_last_of(extension_separator_char);
            if (ext == std::string::npos) {
                p += ".png";
            }
        }

        this->_render_target->save(p);
    }
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
