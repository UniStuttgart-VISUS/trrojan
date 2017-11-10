/// <copyright file="trackball.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/trackball.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"

trrojan::trackball::trackball(std::shared_ptr<camera> cam)
    : _cam(cam)
    , _last_ndc(0.0f)
{
}

void trrojan::trackball::rotate(glm::quat q)
{
    const auto& to = _cam->get_look_to();
    const auto& from = _cam->get_look_from();
    const auto& up = _cam->get_look_up();

    _cam->set_look(_cam->get_look_to() + glm::rotate(q, from - to), to, glm::rotate(q, up));
}

void trrojan::trackball::zoom(float delta)
{
    const auto& to = _cam->get_look_to();
    const auto& from = _cam->get_look_from();

    const vec3 direction = from - to;
    const float direction_length = glm::length(direction);

    if (direction_length)
    {
        const auto zoom = glm::clamp(delta * direction_length, _cam->get_near_plane_dist(),
                                     _cam->get_far_plane_dist());

        _cam->set_look_from(from - glm::normalize(direction) * zoom);
    }
}

void trrojan::trackball::pan(glm::vec2 direction)
{
    const auto prevWorldPos = _cam->get_world_pos_from_ndc(_last_ndc);
    const auto worldPos = _cam->get_world_pos_from_ndc(glm::vec3(direction, 0.f));
    const auto translation = worldPos - prevWorldPos;

    const auto& to = _cam->get_look_to();
    const auto& from = _cam->get_look_from();
    const auto& up = _cam->get_look_up();
    _cam->set_look(from - translation, to - translation, up);
    _last_ndc = glm::vec3(direction, 0.f);
}

void trrojan::trackball::reset()
{
    _cam->reset_look();
}


