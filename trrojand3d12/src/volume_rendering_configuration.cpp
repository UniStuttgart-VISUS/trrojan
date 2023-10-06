// <copyright file="volume_rendering_configuration.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/volume_rendering_configuration.h"

//#include "sphere_techniques.h"
//#include "SpherePipeline.hlsli"


#define _VOLUME_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::volume_rendering_configuration::factor_##f = #f

_VOLUME_BENCH_DEFINE_FACTOR(data_set);
_VOLUME_BENCH_DEFINE_FACTOR(ert_threshold);
_VOLUME_BENCH_DEFINE_FACTOR(frame);
_VOLUME_BENCH_DEFINE_FACTOR(fovy_deg);
_VOLUME_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_VOLUME_BENCH_DEFINE_FACTOR(max_steps);
_VOLUME_BENCH_DEFINE_FACTOR(min_prewarms);
_VOLUME_BENCH_DEFINE_FACTOR(min_wall_time);
_VOLUME_BENCH_DEFINE_FACTOR(step_size);
_VOLUME_BENCH_DEFINE_FACTOR(xfer_func);

#undef _VOLUME_BENCH_DEFINE_FACTOR


//_DEFINE_SPHERE_TECHNIQUE_LUT(SPHERE_METHODS);


///*
// * trrojan::d3d12::volume_rendering_configuration::get_methods
// */
//std::vector<std::string>
//trrojan::d3d12::volume_rendering_configuration::get_methods(void) {
//    std::vector<std::string> retval;
//
//    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr); ++i) {
//        retval.emplace_back(::SPHERE_METHODS[i].name);
//    }
//
//    return retval;
//}
//
//
///*
// * trrojan::d3d12::sphere_benchmark_base::get_shader_id
// */
//trrojan::d3d12::volume_rendering_configuration::shader_id_type
//trrojan::d3d12::volume_rendering_configuration::get_shader_id(
//        const std::string& method) {
//    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr);++i) {
//        if (method == ::SPHERE_METHODS[i].name) {
//            return ::SPHERE_METHODS[i].id;
//        }
//    }
//    /* Not found at this point. */
//
//    return 0;
//}


#define _VOLUME_BENCH_INIT_FACTOR(f)                                           \
    _##f(config.get<decltype(_##f)>(factor_##f))

/*
 * trrojan::d3d12::volume_rendering_configuration::volume_rendering_configuration
 */
trrojan::d3d12::volume_rendering_configuration::volume_rendering_configuration(
        const configuration& config)
    : _VOLUME_BENCH_INIT_FACTOR(data_set),
        _VOLUME_BENCH_INIT_FACTOR(ert_threshold),
        _VOLUME_BENCH_INIT_FACTOR(frame),
        _VOLUME_BENCH_INIT_FACTOR(fovy_deg),
        _VOLUME_BENCH_INIT_FACTOR(gpu_counter_iterations),
        _VOLUME_BENCH_INIT_FACTOR(max_steps),
        _VOLUME_BENCH_INIT_FACTOR(min_prewarms),
        _VOLUME_BENCH_INIT_FACTOR(min_wall_time),
        _VOLUME_BENCH_INIT_FACTOR(step_size),
        _VOLUME_BENCH_INIT_FACTOR(xfer_func) { }

#undef _VOLUME_BENCH_INIT_FACTOR
