// <copyright file="benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Michael Becher</author>

#include "trrojan/d3d11/benchmark_base.h"

#include <chrono>
#include <ctime>

#include "trrojan/executive.h"
#include "trrojan/factor.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/system_factors.h"
#include "trrojan/text.h"

#include "trrojan/d3d11/environment.h"
#include "trrojan/d3d11/bench_render_target.h"
#include "trrojan/d3d11/debug_render_target.h"
#include "trrojan/d3d11/uwp_debug_render_target.h"
#include "trrojan/d3d11/utilities.h"


#define _D3D_BENCH_DEFINE_FACTOR(f)                                         \
const std::string trrojan::d3d11::benchmark_base::factor_##f(#f)

_D3D_BENCH_DEFINE_FACTOR(debug_view);
_D3D_BENCH_DEFINE_FACTOR(save_view);
_D3D_BENCH_DEFINE_FACTOR(sync_interval);

#undef _D3D_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d11::benchmark_base::~benchmark_base
 */
trrojan::d3d11::benchmark_base::~benchmark_base(void) { }


/*
 * trrojan::d3d11::benchmark_base::can_run
 */
bool trrojan::d3d11::benchmark_base::can_run(trrojan::environment env,
        trrojan::device device) const noexcept {
    auto d = std::dynamic_pointer_cast<trrojan::d3d11::device>(device);
    return (d != nullptr);
}


/*
 * trrojan::d3d11::benchmark_base::run
 */
trrojan::result trrojan::d3d11::benchmark_base::run(const configuration& c) {
    std::vector<std::string> changed;
    this->check_changed_factors(c, std::back_inserter(changed));

    auto genericDev = c.get<trrojan::device>(factor_device);
    auto device = std::dynamic_pointer_cast<trrojan::d3d11::device>(genericDev);
    auto powerCollector = initialise_power_collector(c);
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
        log::instance().write_line(log_level::warning, "Using the debug view "
            "restricts the benchmark to the GPU connected to the display. The "
            "device parameter has no effect.");
        if (this->debug_target == nullptr) {
#if defined(TRROJAN_FOR_UWP)
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D11 debug render target.");
            auto uwp_debug_target = std::make_shared<uwp_debug_render_target>();
            uwp_debug_target->set_window(window);
            this->debug_target = uwp_debug_target;
            //this->debug_target->resize(1, 1);   // Force resource allocation.
            this->debug_device = std::make_shared<d3d11::device>(
                [this](void) { return this->debug_target->device(); });
#else /* defined(TRROJAN_FOR_UWP) */
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D11 debug render target.");
            this->debug_target = std::make_shared<debug_render_target>();
            this->debug_target->resize(1, 1);   // Force resource allocation.
            this->debug_device = std::make_shared<d3d11::device>(
                this->debug_target->device());
#endif /* defined(TRROJAN_FOR_UWP) */
        }

        // Overwrite device and render target.
        device = this->debug_device;
        this->render_target = this->debug_target;
        //this->render_target->use_reversed_depth_buffer(true);

    } else {
        // Check whether the device has been changed. This should always be done
        // first, because all GPU resources, which depend on the content of the
        // configuration depend on the device as their storage location.
        if (contains_any(changed, factor_device)) {
            log::instance().write_line(log_level::verbose, "The D3D device has "
                "changed. Reallocating all graphics resources ...");
            this->render_target = std::make_shared<bench_render_target>(device);
            //this->render_target->use_reversed_depth_buffer(true);
            // If the device has changed, force the viewport to be re-created:
            changed.push_back(factor_viewport);
        }
    } /* end if (isDebugView) */

    // Resize the render target if the viewport has changed.
    if (contains(changed, factor_viewport)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        log::instance().write_line(log_level::verbose, "Resizing the "
            "benchmarking render target to {} × {} px ...", vp[0], vp[1]);
        this->render_target->resize(vp[0], vp[1]);
    }

    this->render_target->enable();
    auto retval = this->on_run(*device, c, powerCollector, changed);

    if (c.get<bool>(factor_save_view)) {
        auto ts = c.get<std::string>(system_factors::factor_timestamp);
        std::replace(ts.begin(), ts.end(), ':', '-');
        std::replace(ts.begin(), ts.end(), '.', '-');

        std::string save_path;

#ifdef _UWP
        auto prefix = winrt::to_string(winrt::Windows::Storage::ApplicationData::Current().TemporaryFolder().Path());
        save_path.append(prefix);
        save_path.append("/");
#endif // _UWP

        save_path.append(ts);
        this->save_target(save_path.c_str());
    }

    return retval;
}


/*
 * trrojan::d3d11::benchmark_base::benchmark_base
 */
trrojan::d3d11::benchmark_base::benchmark_base(const std::string& name)
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
 * trrojan::d3d11::benchmark_base::save_target
 */
void trrojan::d3d11::benchmark_base::save_target(const char *path) {
    if (this->render_target != nullptr) {
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

        this->render_target->save(p);
    }
}


/*
 * trrojan::d3d11::benchmark_base::switch_to_uav_target
 */
winrt::com_ptr<ID3D11UnorderedAccessView>
trrojan::d3d11::benchmark_base::switch_to_uav_target(void) {
    assert(this->render_target != nullptr);
    auto ctx = this->render_target->device_context();
    ID3D11RenderTargetView *rtv[] = { nullptr };
    ctx->OMSetDepthStencilState(nullptr, 0);
    ctx->OMSetRenderTargets(1, rtv, nullptr);
    return this->render_target->to_uav();
}
