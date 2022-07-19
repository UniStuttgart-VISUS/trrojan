// <copyright file="clipping.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <utility>

#include "trrojan/camera.h"
#include "trrojan/log.h"


namespace trrojan {

    /// <summary>
    /// Computes reasonable clipping planes for the given bounding box.
    /// </summary>
    /// <typeparam name="TBbox">The type of the bounding box, which must be
    /// an array type of two vectors, both with an indexer. The outer array
    /// must support <c>std::size</c>.</typeparam>
    /// <param name="camera">The camera for which the planes are for. The
    /// camera must be in the final state for the rendering except for the
    /// clipping planes.</param>
    /// <param name="bbox_min">The minimum point of the bounding box in world
    /// space.</param>
    /// <param name="bbox_max">The maximum point of the bounding box in world
    /// space.</param>
    /// <param name="offset">An additional offset subtracted/added from the
    /// computed position, which accounts for things like sphere radii. This
    /// parameter defaults to zero.</param>
    /// <returns>The distance of the near and far clipping planes.</returns>
    template<class TBbox>
    std::pair<float, float> calc_clipping_planes(const camera& camera,
        const TBbox& bbox, const float offset = 0.0f);

    /// <summary>
    /// Computes reasonable clipping planes for the given bounding box and
    /// applies them to the camera.
    /// </summary>
    /// <typeparam name="TBbox">The type of the bounding box, which must be
    /// an array type of two vectors, both with an indexer. The outer array
    /// must support <c>std::size</c>.</typeparam>
    /// <param name="camera">The camera to apply the clipping planes to. The
    /// camera must be in the final state for the rendering except for the
    /// clipping planes.</param>
    /// <param name="bbox_min">The minimum point of the bounding box in world
    /// space.</param>
    /// <param name="bbox_max">The maximum point of the bounding box in world
    /// space.</param>
    /// <param name="offset">An additional offset subtracted/added from the
    /// computed position, which accounts for things like sphere radii. This
    /// parameter defaults to zero.</param>
    /// <returns><paramref name="camera" />.</returns>
    template<class TBbox>
    inline camera& set_clipping_planes(camera& camera, const TBbox& bbox,
            const float offset = 0.0f) {
        auto planes = calc_clipping_planes(camera, bbox, offset);
        camera.set_near_plane_dist(planes.first);
        camera.set_far_plane_dist(planes.second);
        return camera;
    }


    /// <summary>
    /// Sets the field of view and afterwards the clipping planes for the given
    /// data set extents.
    /// </summary>
    /// <typeparam name="TBbox"></typeparam>
    /// <param name="camera"></param>
    /// <param name="fovy"></param>
    /// <param name="bbox"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    template<class TBbox>
    inline camera& set_fovy_and_clipping_planes(camera& camera, float fovy,
            const TBbox& bbox, const float offset = 0.0f) {
        camera.set_fovy(fovy);
        return set_clipping_planes(camera, bbox, offset);
    }
} /* end namespace trrojan */

#include "trrojan/clipping.inl"
