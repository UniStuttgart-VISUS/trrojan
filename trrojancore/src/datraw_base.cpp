// <copyright file="datraw_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/datraw_base.h"

#include <algorithm>
#include <stdexcept>

#include "trrojan/clipping.h"
#include "trrojan/log.h"


/*
 * trrojan::datraw_base::calc_base_step_size
 */
float trrojan::datraw_base::calc_base_step_size(const info_type& info) {
    auto size = info.slice_thickness();
    auto shortest = *(std::min_element)(size.begin(), size.end());
    return (shortest);
}


/*
 * trrojan::datraw_base::calc_physical_size
 */
std::array<float, 3> trrojan::datraw_base::calc_physical_size(
        const info_type& info) {
    auto resolution = info.resolution();
    auto dists = info.contains(info_type::property_slice_thickness)
        ? info.slice_thickness()
        : std::vector<float> { 1.0f, 1.0f, 1.0f };

    if (resolution.size() != 3) {
        throw std::invalid_argument("Only three-dimensional data are "
            "supported.");
    }
    if (resolution.size() != dists.size()) {
        throw std::invalid_argument("The slice distances and the resolution "
            "do not share the same number of components.");
    }

    return std::array<float, 3> {
        static_cast<float>(resolution[0]) * dists[0],
        static_cast<float>(resolution[1]) * dists[1],
        static_cast<float>(resolution[2]) * dists[2]
    };
}


/*
 * trrojan::datraw_base::calc_ray_scale
 */
std::array<float, 3> trrojan::datraw_base::calc_ray_scale(
        const info_type & info) {
    static const std::array<float, 3> unit = { 1.0f, 1.0f, 1.0f };

    if (info.contains(info_type::property_slice_thickness)) {
        auto d = info.slice_thickness();
        if (d.size() != 3) {
            throw std::invalid_argument("Only three-dimensional data are "
                "supported.");
        }

        return std::array<float, 3> { 1.0f / d[0], 1.0f / d[1], 1.0f / d[2] };

    } else {
        return unit;
    }
}


/*
 * trrojan::datraw_base::calc_bounding_box
 */
void trrojan::datraw_base::calc_bounding_box(glm::vec3& outStart,
        glm::vec3& outEnd) const {
    auto size = this->calc_physical_volume_size();
    outStart = glm::vec3(size[0], size[1], size[2]);
    outEnd = outStart;

    for (glm::vec3::length_type i = 0; i < outStart.length(); ++i) {
        outStart[i] *= -0.5f;
        outEnd[i] *= 0.5f;
    }
}


/*
 * trrojan::datraw_base::calc_clipping_planes
 */
std::pair<float, float> trrojan::datraw_base::calc_clipping_planes(
        const camera& cam) const {
    glm::vec3 bbox[2];
    this->calc_bounding_box(bbox[0], bbox[1]);
    return trrojan::calc_clipping_planes(cam, bbox);
}
