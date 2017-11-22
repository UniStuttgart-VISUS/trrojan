/// <copyright file="benchmark_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/benchmark_base.h"

#include "trrojan/factor.h"
#include "trrojan/log.h"

#include "trrojan/d3d11/environment.h"
#include "trrojan/d3d11/bench_render_target.h"
#include "trrojan/d3d11/utilities.h"


#define _D3D_BENCH_DEFINE_FACTOR(f)                                         \
const std::string trrojan::d3d11::benchmark_base::factor_##f(#f)

_D3D_BENCH_DEFINE_FACTOR(debug_view);
_D3D_BENCH_DEFINE_FACTOR(viewport);

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
 * trrojan::d3d11::benchmark_base::get_debug_staging_texture
 */
HANDLE trrojan::d3d11::benchmark_base::get_debug_staging_texture(void) {
    if (this->render_target != nullptr) {
        auto sb = this->render_target->staging_buffer();
        if (sb != nullptr) {
            return get_shared_handle(sb.p);
        }
    }
    
    return NULL;
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

    // Check whether the device has been changed. This should always be done
    // first, because all GPU resources, which depend on the content of 'c',
    // depend on the device as their storage location.
    if (contains(factor_device, changed)) {
        log::instance().write_line(log_level::verbose, "The D3D device has "
            "changed. Reallocating all graphics resources ...");
        this->render_target = std::make_shared<bench_render_target>(device);
        // If the device has changed, force the viewport to be re-created:
        changed.push_back(factor_viewport);
    }

    // Resize the render target if the viewport has changed.
    if (contains(factor_viewport, changed)) {
        auto vp = c.get<viewport_type>(factor_viewport);
        log::instance().write_line(log_level::verbose, "Resizing the "
            "benchmarking render target to %d × %d px ...", vp[0], vp[1]);
        this->render_target->resize(vp[0], vp[1]);
    }

    // Find out whether we need to start the debug view.
    {
        auto dv = c.get<bool>(factor_debug_view);
        auto env = c.get<trrojan::environment>(factor_environment);
        auto d3dEnv = std::dynamic_pointer_cast<d3d11::environment>(env);
        d3dEnv->show_debug_view(*this);

        log::instance().write_line(log_level::verbose, "Configuring the "
            "debug view to be %s ...", dv ? "on" : "off");
        //if (true||dv) {
        //    if (this->debugView == nullptr) {
        //        this->debugView = std::make_shared<debug_view>();
        //    }
        //} else {
        //    this->debugView = nullptr;
        //}
    }

    this->render_target->clear();
    this->render_target->enable();
    return this->on_run(*device, c, changed);
}


/*
 * trrojan::d3d11::benchmark_base::contains
 */
bool trrojan::d3d11::benchmark_base::contains(const std::string& needle,
        const std::vector<std::string>& haystack) {
    auto it = std::find(haystack.begin(), haystack.end(), needle);
    return (it != haystack.end());
}


/*
 * trrojan::d3d11::benchmark_base::benchmark_base
 */
trrojan::d3d11::benchmark_base::benchmark_base(const std::string& name)
        : trrojan::benchmark_base(name) {
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_debug_view, false));

    {
        auto dftViewport = std::array<unsigned int, 2> { 1024, 1024 };
        this->_default_configs.add_factor(factor::from_manifestations(
            factor_viewport, dftViewport));
    }
}
