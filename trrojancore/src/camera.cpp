/// <copyright file="camera.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/camera.h"
#include <stdexcept>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "glm/gtc/constants.hpp"

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

glm::mat4 trrojan::camera::get_view_arcball(const glm::quat q, const float distance)
{
    glm::mat4 qmat = glm::mat4_cast(q);
    qmat = glm::translate(qmat, glm::vec3(0, 0, distance));
    return qmat;
}


glm::vec3 trrojan::camera::get_eye_ray(float x, float y)
{
    glm::vec4 tmp(x, y, 0.01f, 1.f);
    tmp = this->get_inverse_projection_mx() * tmp;
    tmp /= tmp.w;
    glm::vec3 ray = glm::vec3(tmp) - this->_look_from;
    return ray;
}

void trrojan::camera::rotate_fixed_to(const glm::quat q)
{
    _look_from = _look_to - q * (_look_from - _look_to);
    _look_up = q * glm::vec3(0, 1, 0);  // up vector fixed to positive y-axis
}

void trrojan::camera::rotate_fixed_from(const glm::quat q)
{
    // FIXME
    _look_to = q * (_look_from - _look_to);
    _look_up = q * glm::vec3(0, 1, 0);  // up vector fixed to positive y-axis
}

/**
 * trrojan::camera::set_from_maneuver
 */
void trrojan::camera::set_from_maneuver(const std::string &name, const glm::vec3 bbox_min,
                                        const glm::vec3 bbox_max, const int iteration,
                                        const float fovy, const int samples)
{
    if (iteration >= samples)
        throw std::invalid_argument("Iteration must be smaller than #samples.");

    float pi = glm::pi<float>();
    // circle around axis
    if (name.find("orbit") != std::string::npos)
    {
        // look at bounding box center
        this->set_look_to(bbox_min + (bbox_max - bbox_min)*0.5f);
        // fit view to bounding box x
        float bbox_length_x = bbox_max.x - bbox_min.x;
        float camera_dist = (bbox_length_x * 0.5f) / std::tan(pi*fovy*0.5f/180.f);
        this->set_look_from(this->_look_to - glm::vec3(0, 0, bbox_min.z - camera_dist));
        // rotation in radians
        float angle = (2.f*pi / samples) * iteration;
        glm::vec3 axis(0.f);
        // create axis from x,y,z identifier
        if (name.find("x") != std::string::npos) axis.x = 1.f;
        if (name.find("y") != std::string::npos) axis.y = 1.f;
        if (name.find("z") != std::string::npos) axis.z = 1.f;
        if (name.find("-x") != std::string::npos) axis.x *= -1.f;
        if (name.find("-y") != std::string::npos) axis.y *= -1.f;
        if (name.find("-z") != std::string::npos) axis.z *= -1.f;
        glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
        this->rotate_fixed_to(q);
    }
    // fly-through on diagonal
    else if (name.find("diagonal") != std::string::npos)
    {
        glm::vec3 begin = bbox_min;
        glm::vec3 end = bbox_max;
        // point mirroring according to definition (e.g. "diagonal_zx" is between (0,1,1)->(1,0,0))
        if (name.find("z") != std::string::npos) std::swap(begin, end);        // back to front
        if (name.find("x") != std::string::npos) std::swap(begin.x, end.x);    // right to left
        if (name.find("y") != std::string::npos) std::swap(begin.y, end.y);    // top to bottom

        this->set_look_to(end);
        float diagonal_length = glm::distance(bbox_min, bbox_max); // glm::root_two<float>()*
        float camera_dist = (diagonal_length * 0.5f) / std::tan(fovy*0.5f*pi/180.f);
        camera_dist *= (1.f - iteration/(float)samples);
        this->set_look_from(bbox_min + (bbox_max - bbox_min)*0.5f + glm::normalize(begin - end)*camera_dist);
    }
    else if (name.find("path") != std::string::npos)
    {
        // NOTE: curves only implemented in xz-plane
        // fit initial view to bounding box x
        this->set_look_to(bbox_min + (bbox_max - bbox_min)*0.5f);
        float bbox_length_x = bbox_max.x - bbox_min.x;
        float camera_dist = (bbox_length_x * 0.5f) / std::tan(fovy*0.5f*pi/180.f);
        glm::vec3 start = glm::vec3(bbox_min) - glm::vec3(camera_dist);
        float x = 2.f*pi * (iteration / (float)samples);
        float view_translation = (iteration / (float)samples)*(camera_dist + bbox_length_x);
        glm::vec3 to = glm::vec3(0);
        _look_up = glm::vec3(0,1,0);
        start += glm::vec3(view_translation);

        float tangent = bbox_length_x*0.25f; // amplitude
        // straight lines
        if (name.find("x") != std::string::npos)
        {
            this->set_look_from(start*glm::vec3(1,0,0));
            to = glm::vec3(1, 0, 0);
        }
        else if (name.find("y") != std::string::npos)
        {
            this->set_look_up(glm::vec3(1,0,0));
            this->set_look_from(start*glm::vec3(0,1,0));
            to = glm::vec3(0, 1, 0);
        }
        else if (name.find("z") != std::string::npos)
        {
            this->set_look_from(start*glm::vec3(0,0,1));
            to = glm::vec3(0, 0, 1);
        }
        // curves
        // NOTE: currently only sin/cos in xz-plane are supported!
        if (name.find("sin") != std::string::npos)
        {
            this->_look_from += glm::vec3(sin(x)*tangent, 0, 0);
            tangent *= cos(x);
            to = 2.f * glm::vec3(tangent, 0, 1.f - tangent);
        }
        else if (name.find("cos") != std::string::npos)
        {
            this->_look_from -= glm::vec3(cos(x)*tangent, 0, 0);
            tangent *= -sin(x);
            to = glm::vec3(tangent, 0, 1.f - tangent);
        }
        else    // straight line
        {
            to *= glm::vec3(_look_from.z + camera_dist*2.0f);
        }
        this->set_look_to(to - _look_from);
    }
    else if (name.find("random") != std::string::npos)
    {
        this->set_look_to(glm::vec3(bbox_min + (bbox_max - bbox_min)*0.5f));
        // fit view to bounding box x
        float bbox_length_x = bbox_max.x - bbox_min.x;
        float camera_dist = (bbox_length_x * 0.5f) / std::tan(fovy*0.5f*pi/180.f);
        // set random distance between center of bbox and 2x bbox fitted view
        float r = get_rand()*2.f;
        this->set_look_from(this->_look_to - glm::vec3(0, 0, (bbox_min.z - camera_dist)*r));
        // rotate uniform sampled distance on sphere
        glm::quat q = {get_rand(), get_rand(), get_rand(), get_rand()};
        q = glm::normalize(q);
        this->rotate_fixed_to(q);
    }
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

void trrojan::perspective_camera::set_from_maneuver(const std::string &name, const glm::vec3 bbox_min,
                                                    const glm::vec3 bbox_max, const int iteration,
                                                    const int samples)
{
    camera::set_from_maneuver(name, bbox_min, bbox_max, iteration, get_fovy(), samples);
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

void trrojan::orthographic_camera::set_from_maneuver(const std::string &name,
                                                     const glm::vec3 bbox_min,
                                                     const glm::vec3 bbox_max, const int iteration,
                                                     const int samples)
{
    // TODO: handle fovy
    camera::set_from_maneuver(name, bbox_min, bbox_max, iteration, 90.f, samples);
}
