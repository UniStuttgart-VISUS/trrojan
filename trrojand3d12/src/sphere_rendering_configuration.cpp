// <copyright file="sphere_rendering_configuration.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_rendering_configuration.h"

#include "trrojan/executive.h"
#include "trrojan/io.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::sphere_rendering_configuration::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_maximum);
_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_minimum);
_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_scale);
_SPHERE_BENCH_DEFINE_FACTOR(conservative_depth);
_SPHERE_BENCH_DEFINE_FACTOR(data_set);
_SPHERE_BENCH_DEFINE_FACTOR(edge_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(fit_bounding_box);
_SPHERE_BENCH_DEFINE_FACTOR(force_float_colour);
_SPHERE_BENCH_DEFINE_FACTOR(frame);
_SPHERE_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_SPHERE_BENCH_DEFINE_FACTOR(hemi_tess_scale);
_SPHERE_BENCH_DEFINE_FACTOR(inside_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(method);
_SPHERE_BENCH_DEFINE_FACTOR(min_prewarms);
_SPHERE_BENCH_DEFINE_FACTOR(min_wall_time);
_SPHERE_BENCH_DEFINE_FACTOR(poly_corners);
_SPHERE_BENCH_DEFINE_FACTOR(vs_raygen);
_SPHERE_BENCH_DEFINE_FACTOR(vs_xfer_function);

#undef _SPHERE_BENCH_DEFINE_FACTOR


_DEFINE_SPHERE_TECHNIQUE_LUT(SPHERE_METHODS);


/*
 * trrojan::d3d12::sphere_rendering_configuration::get_methods
 */
std::vector<std::string>
trrojan::d3d12::sphere_rendering_configuration::get_methods(void) {
    std::vector<std::string> retval;

    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr); ++i) {
        retval.emplace_back(::SPHERE_METHODS[i].name);
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_shader_id
 */
trrojan::d3d12::sphere_rendering_configuration::shader_id_type
trrojan::d3d12::sphere_rendering_configuration::get_shader_id(
        const std::string& method) {
    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr);++i) {
        if (method == ::SPHERE_METHODS[i].name) {
            return ::SPHERE_METHODS[i].id;
        }
    }
    /* Not found at this point. */

    return 0;
}


#define _SPHERE_BENCH_INIT_FACTOR(f)                                           \
    _##f(config.get<decltype(_##f)>(factor_##f))

/*
 * trrojan::d3d12::sphere_rendering_configuration::sphere_rendering_configuration
 */
trrojan::d3d12::sphere_rendering_configuration::sphere_rendering_configuration(
        const configuration& config)
    : _SPHERE_BENCH_INIT_FACTOR(adapt_tess_maximum),
        _SPHERE_BENCH_INIT_FACTOR(adapt_tess_minimum),
        _SPHERE_BENCH_INIT_FACTOR(adapt_tess_scale),
        _SPHERE_BENCH_INIT_FACTOR(conservative_depth),
        _SPHERE_BENCH_INIT_FACTOR(data_set),
        _SPHERE_BENCH_INIT_FACTOR(edge_tess_factor),
        _SPHERE_BENCH_INIT_FACTOR(fit_bounding_box),
        _SPHERE_BENCH_INIT_FACTOR(force_float_colour),
        _SPHERE_BENCH_INIT_FACTOR(frame),
        _SPHERE_BENCH_INIT_FACTOR(gpu_counter_iterations),
        _SPHERE_BENCH_INIT_FACTOR(hemi_tess_scale),
        _SPHERE_BENCH_INIT_FACTOR(inside_tess_factor),
        _SPHERE_BENCH_INIT_FACTOR(method),
        _SPHERE_BENCH_INIT_FACTOR(min_prewarms),
        _SPHERE_BENCH_INIT_FACTOR(min_wall_time),
        _SPHERE_BENCH_INIT_FACTOR(poly_corners),
        _SPHERE_BENCH_INIT_FACTOR(vs_raygen),
        _SPHERE_BENCH_INIT_FACTOR(vs_xfer_function) {
    try {
        auto it = config.find(executive::factor_data_folder);
        if (it != config.end()) {
            auto f = it->value().get<std::string>();
            this->_data_set = combine_path(f, this->_data_set);
        }
    } catch (...) { /* This is not fatal (data folder has wrong type). */ }
}

#undef _SPHERE_BENCH_INIT_FACTOR


/*
 * trrojan::d3d12::sphere_rendering_configuration::get_tessellation_constants
 */
void trrojan::d3d12::sphere_rendering_configuration::get_tessellation_constants(
        TessellationConstants& dst) const {
    dst.EdgeTessFactor.x = this->_edge_tess_factor[0];
    dst.EdgeTessFactor.y = this->_edge_tess_factor[1];
    dst.EdgeTessFactor.z = this->_edge_tess_factor[2];
    dst.EdgeTessFactor.w = this->_edge_tess_factor[3];

    dst.InsideTessFactor.x = this->_inside_tess_factor[0];
    dst.InsideTessFactor.y = this->_inside_tess_factor[1];

    dst.AdaptiveTessMin = this->_adapt_tess_minimum;
    dst.AdaptiveTessMax = this->_adapt_tess_maximum;
    dst.AdaptiveTessScale = this->_adapt_tess_scale;

    dst.HemisphereTessScaling = this->_hemi_tess_scale;

    dst.PolygonCorners = this->_poly_corners;
}


/*
 * trrojan::d3d12::sphere_rendering_configuration::shader_id
 */
trrojan::d3d12::sphere_rendering_configuration::shader_id_type
trrojan::d3d12::sphere_rendering_configuration::shader_id(void) const {
    auto retval = sphere_rendering_configuration::get_shader_id(this->_method);

    if ((retval & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0) {
        // If the shader is using raycasting, add special flags relevant only
        // for raycasting.
        if (this->_conservative_depth) {
            retval |= SPHERE_VARIANT_CONSERVATIVE_DEPTH;
        }
        if (this->_vs_raygen) {
            retval |= SPHERE_VARIANT_PV_RAY;
        }
    }

    if ((retval & SPHERE_TECHNIQUE_USE_SRV) != 0) {
        // If the shader is using a shader resource view for the data, the
        // floating point conversion flag is relevant. Note that this flag
        // set speculatively and might need to be removed if the data does
        // not contain per-sphere colours.
        if (this->_force_float_colour) {
            retval |= SPHERE_INPUT_FLT_COLOUR;
        }
    }

    // Set the location of the transfer function lookup unconditionally,
    // because we do not know whether the data could require this. We need to
    // erase this flag later if the data do not need it.
    if (this->_vs_xfer_function) {
        retval |= SPHERE_INPUT_PV_INTENSITY;
    } else {
        retval |= SPHERE_INPUT_PP_INTENSITY;
    }

    return retval;
}
