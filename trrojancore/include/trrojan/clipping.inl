// <copyright file="clipping.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::calc_clipping_planes
 */
template<class TBbox>
std::pair<float, float> trrojan::calc_clipping_planes(const camera& camera,
        const TBbox& bbox, const float offset) {
    const auto& cam_pos = camera.get_look_from();
    const auto& look_at = camera.get_look_to();
    const auto view = glm::normalize(look_at - cam_pos);

    auto retval = std::make_pair((std::numeric_limits<float>::max)(),
        std::numeric_limits<float>::lowest());

    // Determine the closest and farthest point from the camera.
    for (std::size_t x = 0; x < std::size(bbox); ++x) {
        for (std::size_t y = 0; y < std::size(bbox); ++y) {
            for (std::size_t z = 0; z < std::size(bbox); ++z) {
                auto pt = glm::vec3(bbox[x][0], bbox[y][1], bbox[z][2]);
                auto ray = pt - cam_pos;
                auto dist = glm::dot(view, ray);
                if (dist < retval.first) {
                    retval.first = dist;
                }
                if (dist > retval.second) {
                    retval.second = dist;
                }
            }
        }
    }

    // Add the user-specified safety margin.
    retval.first -= offset;
    retval.second += offset;

    // TODO: Hot-fix something going terribly wrong with the proj mat.
    if (/*true ||*/ retval.first < 0.0f) {
        // Plane could become negative in data set, which is illegal. A range of
        // 10k seems to be something our shaders can still handle.
        retval.first = retval.second / 10000.0f;
    }
    //retval.first = 0.01f;
    //retval.second *= 1.1f;

    log::instance().write_line(log_level::debug, "Dynamic clipping planes are "
        "located at {} and {}.", retval.first, retval.second);
    return retval;
}
