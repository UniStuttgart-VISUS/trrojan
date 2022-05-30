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
    float3 camPos = viewInvMatrix._41_42_43;
    float3 camDir = viewInvMatrix._21_22_23;

    float3 v = normalize(objPos - camPos);
    float3 u = normalize(camDir);
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));

    float4x4 retval = float4x4(
        float4(r, 0.0f),
        float4(u, 0.0f),
        float4(v, 0.0f),
        float4(0.0f.xxx, 1.0f));

#if 0
    // http://richiesams.blogspot.de/2014/05/hlsl-turning-float4s-into-float4x4.html
    // Hemisphere-hack:
    float4x4 matOrient = float4x4(
        float4(r, 0.0f),
        -float4(v, 0.0f),
        float4(u, 0.0f),
        float4(0.0f.xxx, 1.0f));
#endif

    return retval;
}
