/// <copyright file="trackball.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/export.h"
#include "camera.h"

#include "glm/glm.hpp"

#include <memory>

namespace trrojan
{

/// <summary>
/// Trackball class to rotate and move a camera around a sphere.
/// </summary>
class TRROJANCORE_API trackball
{
public:
    /// <summary>
    /// Initialises a new instance.
    /// </summary>
    /// <parameter name="cam">The camera to operate on.</parameter>
    trackball(std::shared_ptr<camera> cam);

    /// <summary>
    /// Finalises the instance.
    /// </summary>
    ~trackball() = default;

    /// <summary>
    /// Rotate around the focus based on a quaternion.
    /// </summary>
    /// <parameter name="q">The quaternion defining the rotation.</parameter>
    void rotate(glm::quat q);

    /// <summary>
    /// Zoom in/out.
    /// </summary>
    /// <parameter name="delta">The signed zoom distance.</parameter>
    void zoom(float delta);

    /// <summary>
    /// Pan the camera in horizontal/vertical direction.
    /// </summary>
    /// <parameter name="pan">The horizontal/vertical distance to pan.</parameter>
    void pan(glm::vec2 direction);

    /// <summary>
    /// Reset the trackball.
    /// </summary>
    void reset();

protected:
    /// <summary>
    /// The camera to manipulate.
    /// </summary>
    std::shared_ptr<camera> _cam;

    glm::vec3 _last_ndc;
};

}   // namespace trrojan
