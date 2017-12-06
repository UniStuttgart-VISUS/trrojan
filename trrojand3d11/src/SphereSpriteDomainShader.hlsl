/// <copyright file="SphereSpriteHullShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"


// The number of control points, which is one per splat.
#define CNT_CONTROL_POINTS (1)


/// <summary>
/// Hull shader for creating a splat for later raycasting.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
[domain("quad")]
PsInput Main(OutputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        HsConstants constants,
        float2 uv : SV_DomainLocation) {
    PsInput retval = (PsInput) 0;

    const float TWO_PI = 2.0f * 3.14159265358979323846f;
#ifdef HOLOMOL
    const uint eye = patch[0].Eye;
#else /* HOLOMOL */
    const uint eye = 0;
#endif /* HOLOMOL */

    // Select the right matrices.
    float4x4 mvp = ViewProjMatrix[eye];
    float4x4 pm = ProjMatrix[eye];
    float4x4 vm = ViewMatrix[eye];
    float4x4 vmInv = ViewInvMatrix[eye];
    
    // Reconstruct the camera system.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, vmInv);

    // Get the world-space centre of the sphere.
    const float4 pos = patch[0].Position;

    // Re-combine the spheres parameters into a float + colour.
    retval.SphereParams = float4(pos.xyz, patch[0].Radius);
    retval.Colour = patch[0].Colour;
#ifdef HOLOMOL
    retval.Eye = eye;
#endif /* HOLOMOL */

    // If we use the radius of the sphere as size of the triangle fan, its hull
    // are the secants of the final sphere, but we need to have the tangent.
    // Adjust the value such that the radius is equal to the altitude (apothem)
    // of the triangle.
    float rad = patch[0].Radius;
    float alpha = TWO_PI / (2.0f * constants.EdgeTessFactor[0]);
    rad /= cos(alpha);

    // Compute polar coordinates for the fan around the sphere.
    float phi = TWO_PI * uv.x;
    float sinPhi, cosPhi;
    sincos(phi, sinPhi, cosPhi);
    float4 coords = float4(rad * cosPhi, rad * sinPhi, 0.0f, 0.0f);

    float3 v = normalize((pos - retval.CameraPosition).xyz);
    float3 u = normalize(vmInv._21_22_23);
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));
    float4x4 matOrient = float4x4(
        float4(r, 0.0f),
        float4(u, 0.0f),
        float4(v, 0.0f),
        float4(0.0f.xxx, 1.0f));
    coords = mul(coords, matOrient);

    // Move the fan to world coordinates.
    coords += pos;
    //coords -= float4(rad * v, 0.0f);

    // Perform projection.
    retval.Position = mul(coords, mvp);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos.xyz;

#if 0
    float pi2 = 6.28318530;
    float pi = pi2 / 2.0f;
    float R = patch[0].Radius;
    float fi = pi * uv.x;
    float theta = pi2 * uv.y;
    float sinFi, cosFi, sinTheta, cosTheta;
    sincos(fi, sinFi, cosFi);
    sincos(theta, sinTheta, cosTheta);

    float3 spherePosition = float3(R * sinFi * cosTheta, R * sinFi * sinTheta, R * cosFi);

    float4 pos = float4(spherePosition, 1.0f);

    pos += patch[0].Position;
    pos = path[0].Position + float4(10.0f * uv, 0.0f, 0.0f);
    pos = mul(pos, ViewMatrix);

    // Store the world position.
    //retval.worldPos = (min16float3)pos;

    // Correct for perspective and project the vertex position onto the screen.
    pos = mul(pos, ViewProjMatrix);
    retval.Position = (min16float4)pos;

    // Calculate the normal by applying only the rotation parts of the transform.
    //float3 normal = mul(spherePosition, (float3x3)normalsToWorld);
    //retval.worldNorm = (min16float3)normal;

    //retval.color = (min16float3)(normalize(spherePosition) + 0.4);
    retval.Colour = patch[0].Colour;
    //retval.instId = patch[0].instId;
#endif

    return retval;
}
