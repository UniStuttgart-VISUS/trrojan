/// <copyright file="benchmark_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/benchmark_base.h"

#include <ctime>

#include "trrojan/factor.h"
#include "trrojan/log.h"

#include "trrojan/d3d11/environment.h"
#include "trrojan/d3d11/bench_render_target.h"
#include "trrojan/d3d11/debug_render_target.h"
#include "trrojan/d3d11/utilities.h"


#define _D3D_BENCH_DEFINE_FACTOR(f)                                         \
const std::string trrojan::d3d11::benchmark_base::factor_##f(#f)

_D3D_BENCH_DEFINE_FACTOR(debug_view);

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
        if (this->debug_target == nullptr) {
            log::instance().write_line(log_level::verbose, "Lazy creation of "
                "D3D11 debug render target.");
            this->debug_target = std::make_shared<debug_render_target>();
            this->debug_target->resize(1, 1);   // Force resource allocation.
            this->debug_device = std::make_shared<d3d11::device>(
                this->debug_target->device());
        }

        // Overwrite device and render target.
        device = this->debug_device;
        this->render_target = this->debug_target;

    } else {
        // Check whether the device has been changed. This should always be done
        // first, because all GPU resources, which depend on the content of the
        // configuration depend on the device as their storage location.
        if (contains_any(changed, factor_device)) {
            log::instance().write_line(log_level::verbose, "The D3D device has "
                "changed. Reallocating all graphics resources ...");
            this->render_target = std::make_shared<bench_render_target>(device);
            // If the device has changed, force the viewport to be re-created:
            changed.push_back(factor_viewport);
        }
    } /* end if (isDebugView) */

    // Resize the render target if the viewport has changed.
    if (contains(changed, factor_viewport)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        log::instance().write_line(log_level::verbose, "Resizing the "
            "benchmarking render target to %d × %d px ...", vp[0], vp[1]);
        this->render_target->resize(vp[0], vp[1]);
    }

    this->render_target->enable();
    auto retval = this->on_run(*device, c, changed);

    auto benchTarget = std::dynamic_pointer_cast<bench_render_target>(
        this->render_target);
    if (benchTarget != nullptr) {
        benchTarget->save("honcho.png");
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

        if (path == nullptr) {
            std::vector<char> buffer;
            struct tm tm;
            auto time = ::time(nullptr);
            ::localtime_s(&tm, &time);

            buffer.resize(128);
            ::strftime(buffer.data(), buffer.size(), "%Y%m%d%H%M%S.png", &tm);
            buffer.back() = static_cast<char>(0);

            p = buffer.data();
        } else {
            p = path;
        }

        this->render_target->save(p);
    }
}
