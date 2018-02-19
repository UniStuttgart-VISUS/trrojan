/// <copyright file="sphere_data_set.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_data_set.h"


/*
 * trrojan::d3d11::sphere_data_set_base::centre
 */
trrojan::d3d11::sphere_data_set_base::point_type
trrojan::d3d11::sphere_data_set_base::centre(void) const {
    point_type bbox[2];
    point_type retval;

    this->bounding_box(bbox[0], bbox[1]);

    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] = std::abs(bbox[1][i] - bbox[0][i]);
        retval[i] /= 2.0f;
        retval[i] += std::min(bbox[0][i], bbox[1][i]);
    }

    return retval;
}


/*
 * trrojan::d3d11::sphere_data_set_base::clipping_planess
 */
std::pair<float, float> trrojan::d3d11::sphere_data_set_base::clipping_planes(
        const camera& cam) const {
    point_type bbox[2];
    auto& camPos = cam.get_look_from();
    glm::vec3 centre;
    auto diagLen = 0.0f;
    auto size = this->extents();

    this->bounding_box(bbox[0], bbox[1]);

    for (glm::vec3::length_type i = 0; i < size.size(); ++i) {
        centre[i] = size[i] / 2.0f + bbox[0][i];
        diagLen += size[i] * size[i];
    }
    diagLen = std::sqrt(diagLen);

    diagLen += 0.5f * this->max_radius();
    diagLen *= 0.5f;

    auto viewLen = glm::length(centre - camPos);

    //auto nearPlane = viewLen - diagLen;
    //if (nearPlane < 0.01f) {
    //    nearPlane = 0.01f;
    //}
    auto nearPlane = 0.01f;

    auto farPlane = viewLen + diagLen;
    if (farPlane < nearPlane) {
        farPlane = nearPlane + 1.0f;
    }

    return std::make_pair(nearPlane, farPlane);
}


/*
 * trrojan::d3d11::sphere_data_set_base::extents
 */
std::array<float, 3> trrojan::d3d11::sphere_data_set_base::extents(void) const {
    point_type bbox[2];
    decltype(this->extents()) retval;

    this->bounding_box(bbox[0], bbox[1]);
    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] = std::abs(bbox[1][i] - bbox[0][i]);
    }

    return retval;
}
