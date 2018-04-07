/// <copyright file="graphics_benchmark_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/graphics_benchmark_base.h"

#include <stdexcept>

#include "trrojan/camera.h"


#define _GRAPH_BENCH_BASE_DEFINE_FACTOR(f)                                     \
const std::string trrojan::graphics_benchmark_base::factor_##f(#f)

_GRAPH_BENCH_BASE_DEFINE_FACTOR(manoeuvre);
_GRAPH_BENCH_BASE_DEFINE_FACTOR(manoeuvre_step);
_GRAPH_BENCH_BASE_DEFINE_FACTOR(manoeuvre_steps);
_GRAPH_BENCH_BASE_DEFINE_FACTOR(viewport);

#undef _GRAPH_BENCH_BASE_DEFINE_FACTOR


/*
 * trrojan::graphics_benchmark_base::apply_manoeuvre
 */
void trrojan::graphics_benchmark_base::apply_manoeuvre(trrojan::camera& camera,
        const configuration& config, const point_type& bboxMin,
        const point_type& bboxMax) {
    glm::vec3 bbe(bboxMax[0], bboxMax[1], bboxMax[2]);
    glm::vec3 bbs(bboxMin[0], bboxMin[1], bboxMin[2]);
    auto curStep = config.get<manoeuvre_step_type>(factor_manoeuvre_step);
    auto manoeuvre = config.get<manoeuvre_type>(factor_manoeuvre);
    auto totalSteps = config.get<manoeuvre_step_type>(factor_manoeuvre_steps);

    auto wtf1 = dynamic_cast<perspective_camera *>(&camera);
    auto wtf2 = dynamic_cast<orthographic_camera *>(&camera);

    if (wtf1 != nullptr) {
        wtf1->set_from_maneuver(manoeuvre, bbs, bbe, curStep, totalSteps);
    } else if (wtf2 != nullptr) {
        wtf2->set_from_maneuver(manoeuvre, bbs, bbe, curStep, totalSteps);
    } else {
        throw std::logic_error("Epic WTF!");
    }
}


/*
 * trrojan::graphics_benchmark_base::get_manoeuvre
 */
void trrojan::graphics_benchmark_base::get_manoeuvre(
        manoeuvre_type& outManoeuvre, manoeuvre_step_type& outCurStep,
        manoeuvre_step_type& outTotalSteps, const configuration& config) {
    outManoeuvre = config.get<manoeuvre_type>(factor_manoeuvre);
    outCurStep = config.get<manoeuvre_step_type>(factor_manoeuvre_step);
    outTotalSteps = config.get<manoeuvre_step_type>(factor_manoeuvre_steps);
}


/*
 * trrojan::graphics_benchmark_base::add_default_manoeuvre
 */
void trrojan::graphics_benchmark_base::add_default_manoeuvre(void) {
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre, manoeuvre_type("diagonal")));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_step, static_cast<manoeuvre_step_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_steps, static_cast<manoeuvre_step_type>(64)));
}
