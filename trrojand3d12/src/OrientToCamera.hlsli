// <copyright file="OrientToCamera.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// Computes a matrix to orient a sprite or hemisphere towards the camera.
/// </summary>
/// <param name="objPos">The world-space position of the sprite to be rotated
/// towards the camera.</param>
/// <param name="viewInvMatrix">The inverse view matrix to reconstruct the
/// camera position and orientation.</param>
/// <returns>A matrix orienting the sprite towards the camera.</returns>
float4x4 OrientToCamera(const in float3 objPos,
        const in float4x4 viewInvMatrix) {
    float3 camPos = viewInvMatrix._14_24_34;
    float3 camUp = viewInvMatrix._12_22_32;

    float3 v = normalize(objPos - camPos);
    float3 u = normalize(camUp);
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));

    // http://richiesams.blogspot.de/2014/05/hlsl-turning-float4s-into-float4x4.html
    float4x4 retval = float4x4(
        r.x, u.x, v.x, 0.0f,
        r.y, u.y, v.y, 0.0f,
        r.z, u.z, v.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);

    return retval;
}
