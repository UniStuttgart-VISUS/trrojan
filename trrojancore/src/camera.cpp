/// <copyright file="configuration.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright ? 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/camera.h"


trrojan::camera::camera(vec3 look_from, vec3 look_to, vec3 look_up, float near_plane,
                        float far_plane)
    : _look_from(look_from)
    , _look_to(look_to)
    , _look_up(look_up)
    , _near_plane_dist(near_plane)
    , _far_plane_dist(far_plane)
    , _invalid_view_mx(true)
    , _invalid_projection_mx(true)
{
}

const glm::mat4& trrojan::camera::get_view_mx() const
{
    if (_invalid_view_mx)
    {
        _view_mx = glm::lookAt(_look_from, _look_to, _look_up);
        _inverse_view_mx = glm::inverse(_view_mx);
        _invalid_view_mx = false;
    }
    return _view_mx;
}

const glm::mat4& trrojan::camera::get_projection_mx() const
{
    if (_invalid_projection_mx)
    {
        _projection_mx = calc_projection_mx();
        _inverse_projection_mx = glm::inverse(_projection_mx);
        _invalid_projection_mx = false;
    }
    return _projection_mx;
}

const glm::mat4& trrojan::camera::get_inverse_view_mx() const
{
    if (_invalid_view_mx) get_view_mx();
    return _inverse_view_mx;
}

const glm::mat4& trrojan::camera::get_inverse_projection_mx() const
{
    if (_invalid_projection_mx) get_projection_mx();
    return _inverse_projection_mx;
}

glm::vec3 trrojan::camera::get_world_pos_from_ndc(const vec3& ndc) const
{
    vec4 clip_coords = get_clip_pos_from_ndc(ndc);
    vec4 eye_coords = get_inverse_projection_mx() * clip_coords;
    vec4 world_coords = get_inverse_view_mx() * eye_coords;
    world_coords /= world_coords.w;
    return vec3(world_coords);
}

glm::vec4 trrojan::camera::get_clip_pos_from_ndc(const vec3& ndc) const
{
    const auto& projection = get_projection_mx();
    const float clipW = projection[2][3] / (ndc.z - (projection[2][2] / projection[3][2]));
    return vec4(ndc * clipW, clipW);
}

glm::vec3 trrojan::camera::get_ndc_from_normalized_screen_at_focus_point_depth(
    const vec2& normalized_screen_coords) const
{
    // Default to using focus point for depth
    vec4 look_to_clip_coords = get_projection_mx() * get_view_mx() * vec4(get_look_to(), 1.f);
    return vec3(2.f * normalized_screen_coords - 1.f, look_to_clip_coords.z / look_to_clip_coords.w);
}


// Perspective camera
trrojan::perspective_camera::perspective_camera(vec3 look_from, vec3 look_to, vec3 look_up,
                                                float near_plane, float far_plane,
                                                float field_of_view, float aspect_ratio)
    : camera(look_from, look_to, look_up, near_plane, far_plane)
    , _fovy(field_of_view)
    , _aspect_ratio(aspect_ratio)
{
}


// Orthographic camera
trrojan::orthographic_camera::orthographic_camera(vec3 look_from, vec3 look_to, vec3 look_up,
                                                  float near_plane, float far_plane, vec4 frustum)
    : camera(look_from, look_to, look_up, near_plane, far_plane), _frustum(frustum)
{
}

float trrojan::orthographic_camera::get_aspect_ratio() const
{
    // Left, right, bottom, top view volume
    const float width{_frustum.y - _frustum.x};
    const float height{_frustum.w - _frustum.z};
    return width / height;
}

void trrojan::orthographic_camera::set_aspect_ratio(float val)
{
    // Left, right, bottom, top view volume
    const float width{_frustum.y - _frustum.x};
    const float height{width / val};
    _frustum.z = -height / 2.0f;
    _frustum.w = +height / 2.0f;
    invalidate_projection_mx();
}
