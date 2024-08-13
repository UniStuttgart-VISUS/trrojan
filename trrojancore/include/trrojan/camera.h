/// <copyright file="camera.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
// TODO: THE CAMERA SHOULD BE REWRITTEN TO BE USED POLYMORPHICALLY. The desired
// Implementation would be:
//
// class camera_base { };
// class perspective_camera : public camera_base { };
// class orthographic_camera : public camera_base { };
// typedef std::shared_ptr<camera> camera;
//
// camera_base should contain pure virtual implementations for the manoeuvres.
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <random>

#include "trrojan/export.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

namespace trrojan
{
    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    using glm::mat4;

/// <summary>
/// Base class for a camera.
/// </summary>
class TRROJANCORE_API camera
{
public:
    /// <summary>
    /// Default parameters creates a right handed coordinate system
    /// with camera looking towards the negative z-axis.
    /// with X = (1, 0, 0), Y = (0, 1, 0), Z = (0, 0, -1)
    /// </summary>
    /// <param name="_look_from">Camera position (eye).</param>
    /// <param name="_look_to"> Camera focus point (center).</param>
    /// <param name="_look_up">Camera up direction.</param>
    camera(vec3 look_from = vec3(0.0f, 0.0f, -2.0f), vec3 look_to = vec3(0.0f),
           vec3 look_up = vec3(0.0f, 1.0f, 0.0f), float near_plane = 0.01f,
           float far_plane = 10000.0f);
    virtual ~camera() = default;
    camera(const camera& other) = default;

    void set_look(vec3 from, vec3 to, vec3 up);

    /// <summary>
    /// Reset look to default parameters.
    /// </summary>
    void reset_look();

    const vec3& get_look_from() const;
    void set_look_from(vec3 val);

    const vec3& get_look_to() const;
    void set_look_to(vec3 val);

    const vec3& get_look_up() const;
    void set_look_up(vec3 val);

    virtual float get_aspect_ratio() const = 0;
    virtual void set_aspect_ratio(float val) = 0;

    /// <summary>
    /// Get unnormalized direction of camera: look_to - _look_from
    /// </summary>
    vec3 get_direction() const;

    /// <summary>
    /// Set the distance of _look_from to the near plane.
    /// </summary>
    void set_near_plane_dist(float distance);
    float get_near_plane_dist() const;

    /// <summary>
    /// Set the distance of _look_from to the far plane.
    /// </summary>
    void set_far_plane_dist(float distance);
    float get_far_plane_dist() const;

    /// <summary>
    /// Compute the projection matrix with a NDC z-range of [0, 1].
    /// </summary>
    /// <remarks>
    /// This is the kind of matrix you want to use for the standard D3D graphics
    /// pipeline.
    /// </remarks>
    virtual glm::mat4 calc_projection_mxz0(void) const = 0;

    const mat4& get_view_mx() const;
    const mat4& get_projection_mx() const;
    const mat4& get_inverse_view_mx() const;
    const mat4& get_inverse_projection_mx() const;

    /// <summary>
    /// Convert from normalized device coordinates (xyz in [-1 1]) to world coordinates.
    /// </summary>
    /// <param name="ndc">Coordinates in [-1 1].</param>
    /// <return>World space position.<return>
    vec3 get_world_pos_from_ndc(const vec3& ndc) const;

    /// <summary>
    /// Convert from normalized device coordinates (xyz in [-1 1]) to clip coordinates,
    /// where z=-1 corresponds to the near plane and z=1 to the far plane.
    /// Cooridnates outside range [-1 1]³ will be clipped.
    /// </summary>
    /// <param name="ndc">xyz clip-coordinates in [-1 1]^3, and the clip w-coordinate used for
    /// perspective division.</param>
    /// <return>Clip space position.<return>
    vec4 get_clip_pos_from_ndc(const vec3& ndcCoords) const;

    vec3 get_ndc_from_normalized_screen_at_focus_point_depth(
            const vec2& normalized_screen_coords) const;

    /// <summary>
    /// Calculate the view matrix of a arcball camera (center at (0,0,0)) based on
    /// rotation and distance to the center.
    /// </summary>
    /// <param name="q">Quaternion representing the rotation.</param>
    /// <param name="translation">The transation distance to the center.</param>
    /// <return>the view matrix.<return>
    glm::mat4 get_view_arcball(const glm::quat q, const float distance);

    glm::vec3 get_eye_ray(float x, float y);

    /// <summary>
    /// Rotate the camera around a fixed look_to position.
    /// Note that the resulting vector is not normalized.
    /// </summary>
    /// <param name="q">The quaternion used for rotation</param>
    void rotate_fixed_to(const glm::quat q);

    /// <summary>
    /// Rotate the camera direction, with a fixed camera look_from position.
    /// Note that the resulting vector is not normalized.
    /// </summary>
    /// <param name="q">The quaternion used for rotation</param>
    void rotate_fixed_from(const glm::quat q);

protected:
    virtual mat4 calc_projection_mx() const = 0;
    void invalidate_view_mx();
    void invalidate_projection_mx();
    void set_from_maneuver(const std::string &name, const glm::vec3 bbox_min,
                           const glm::vec3 bbox_max, const int iteration, const float fovy,
                           const int samples = 360);

    vec3 _look_from;
    vec3 _look_to;
    vec3 _look_up;

    float _near_plane_dist;  ///< Distance to the near plane from _look_from.
    float _far_plane_dist;   ///< Distance to the far plane from _look_from.

    mutable bool _invalid_view_mx;
    mutable bool _invalid_projection_mx;
    mutable mat4 _view_mx;
    mutable mat4 _projection_mx;
    mutable mat4 _inverse_view_mx;
    mutable mat4 _inverse_projection_mx;
};


/// <summary>
/// Camera with perspective projection.
/// </summary>
class TRROJANCORE_API perspective_camera : public camera
{
public:
    perspective_camera(vec3 look_from = vec3(0.0f, 0.0f, 2.0f), vec3 look_to = vec3(0.0f),
                       vec3 look_up = vec3(0.0f, 1.0f, 0.0f), float near_plane = 0.01f,
                       float far_plane = 10000.0f, float field_of_view = 90.f, float aspect_ratio = 1.f);
    virtual ~perspective_camera() = default;
    perspective_camera(const perspective_camera& other) = default;

    float get_fovy() const;
    void set_fovy(float val);
    virtual float get_aspect_ratio() const override;
    virtual void set_aspect_ratio(float val) override;

    /// <inheritdoc />
    glm::mat4 calc_projection_mxz0(void) const override;

    void set_from_maneuver(const std::string &name, const glm::vec3 bbox_min,
                           const glm::vec3 bbox_max, const int iteration, const int samples);
protected:
    virtual mat4 calc_projection_mx() const override;

    float _fovy;
    float _aspect_ratio;
};


/// <summary>
/// Camera with no perspective projection.
/// </summary>
class TRROJANCORE_API orthographic_camera : public camera
{
public:
    orthographic_camera(vec3 _look_from = vec3(0.0f, 0.0f, 2.0f), vec3 _look_to = vec3(0.0f),
                        vec3 _look_up = vec3(0.0f, 1.0f, 0.0f), float _near_plane = 0.01f,
                        float _far_plane = 10000.0f, vec4 frustum = vec4(-01, 10, -10, 10));
    virtual ~orthographic_camera() = default;
    orthographic_camera(const orthographic_camera& other) = default;

    const vec4& get_frustum() const;

    /// <summary>
    /// Set view frustum used for projection matrix calculation.
    /// </summary>
    /// <param name="val"> Left, right, bottom, top view volume. </param>
    void set_frustum(vec4 val);
    virtual float get_aspect_ratio() const override;
    virtual void set_aspect_ratio(float val) override;

    /// <inheritdoc />
    glm::mat4 calc_projection_mxz0(void) const override;

    void set_from_maneuver(const std::string& name, const glm::vec3 bbox_min,
                           const glm::vec3 bbox_max, const int iteration, const int samples);
protected:
    virtual mat4 calc_projection_mx() const override;

    vec4 _frustum; ///< Left, right, bottom, top view volume.
};


// Implementation
inline const vec3& camera::get_look_from() const { return _look_from; }
inline const vec3& camera::get_look_to() const { return _look_to; }
inline const vec3& camera::get_look_up() const { return _look_up; }

inline void camera::set_look(glm::vec3 from, glm::vec3 to, glm::vec3 up)
{
    _look_from = from;
    _look_to = to;
    _look_up = up;
    invalidate_view_mx();
}

inline void camera::reset_look()
{
    _look_from = vec3(0.0f, 0.0f, 2.0f);
    _look_to = vec3(0.0f);
    _look_up = vec3(0.0f, 1.0f, 0.0f);
    invalidate_view_mx();
}

inline void camera::set_look_from(vec3 val)
{
    _look_from = val;
    invalidate_view_mx();
}

inline void camera::set_look_to(vec3 val)
{
    _look_to = val;
    invalidate_view_mx();
}

inline void camera::set_look_up(vec3 val)
{
    _look_up = val;
    invalidate_view_mx();
}

inline vec3 camera::get_direction() const { return _look_to - _look_from; }

inline float camera::get_near_plane_dist() const { return _near_plane_dist; }
inline void camera::set_near_plane_dist(float val)
{
    _near_plane_dist = val;
    invalidate_projection_mx();
}
inline float camera::get_far_plane_dist() const { return _far_plane_dist; }
inline void camera::set_far_plane_dist(float val)
{
    _far_plane_dist = val;
    invalidate_projection_mx();
}

inline void camera::invalidate_view_mx() { _invalid_view_mx = true; }
inline void camera::invalidate_projection_mx() { _invalid_projection_mx = true; }


// Perspective camera
inline float perspective_camera::get_fovy() const { return _fovy; }
inline void perspective_camera::set_fovy(float val)
{
    _fovy = val;
    invalidate_projection_mx();
}

inline float perspective_camera::get_aspect_ratio() const { return _aspect_ratio; }
inline void perspective_camera::set_aspect_ratio(float val) {
    _aspect_ratio = val;
    invalidate_projection_mx();
}


inline mat4 perspective_camera::calc_projection_mx() const
{
    return glm::perspective(glm::radians(_fovy), _aspect_ratio, _near_plane_dist, _far_plane_dist);
}


// Orthographic camera
inline const vec4& orthographic_camera::get_frustum() const { return _frustum; }
inline void orthographic_camera::set_frustum(vec4 val)
{
    _frustum = val;
    invalidate_projection_mx();
}

inline mat4 orthographic_camera::calc_projection_mx() const
{
    return glm::ortho(_frustum.x, _frustum.y, _frustum.z, _frustum.w, _near_plane_dist,
                      _far_plane_dist);
}


}   // namespace trrojan
