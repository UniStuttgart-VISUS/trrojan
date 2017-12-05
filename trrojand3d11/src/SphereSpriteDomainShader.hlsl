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
VsOutput Main(OutputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
//PsInput Main(OutputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        HsConstants constants,
        float2 uv : SV_DomainLocation) {
    //PsInput retval = (PsInput) 0;
    VsOutput retval;

    const float PI = 3.14159265358979323846f;

    // Select the right matrices.
    float4x4 pm = ProjMatrix;
    float4x4 vm = ViewMatrix;
    float4x4 vmInv = ViewInvMatrix;
    

    // Reconstruct the camera system.
    //ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
    //    retval.CameraUp, retval.CameraRight, vmInv);

    // Re-combine the spheres parameters into a float + colour.
    //retval.SphereParams = float4(patch[0].Position.xyz, patch[0].Radius);
    //retval.Colour = patch[0].Colour;

#if 1
    // Transform the position to camera coordinates.
    float4 pos = mul(patch[0].Position, vm);
    float rad = patch[0].Radius;

    float phi = 2.0f * PI * uv.x;
    float sinPhi, cosPhi;
    sincos(phi, sinPhi, cosPhi);

    float4 coords = float4(rad * cosPhi, rad * sinPhi, 0.0f, 1.0f);
    pos += coords;

    retval.Position = mul(pos, pm);
    retval.Radius = patch[0].Radius;
    retval.Colour = patch[0].Colour;
#endif

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
