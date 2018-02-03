/// <copyright file="SphereVertexShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "LocalLighting.hlsli"
#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"


/// <summary>
/// Vertex shader for constant radius and colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = IntRangeGlobalRadTessFactor.z;
    retval.Colour = GlobalColour;
#ifdef HOLOMOL
    retval.Eye = input.Eye;
#endif /* HOLOMOL */

    return retval;
}


#if 0
/// <summary>
/// Vertex shader for a quad sprite from nothing
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
PsInput Main(uint instanceID : SV_InstanceID, uint vertexID : SV_VertexID) {
    PsInput retval = (PsInput) 0;

    uint particleID = instanceID;
#ifdef HOLOMOL
    uint eye = particleID % 2;
    particleID /= 2;
#else /* HOLOMOL */
    uint eye = 0;
#endif /* HOLOMOL */

    // Make the right vertex.
    const float2 FLIP_Y = float2(1.0f, -1.0f);  // Flips the y-axis of a float2.
    retval.Position = float4(vertexID % 2, vertexID % 4 / 2, 0.0f, 1.0f);
    retval.Position.xy = 2.0f * FLIP_Y * (retval.Position.xy - 0.5f.xx);

    // Select the right matrices.
    //float4x4 pm = ProjMatrix[eye];
    //float4x4 vm = ViewMatrix[eye];
    float4x4 mvp = ViewProjMatrix[eye];

    // Reconstruct the camera system.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, ViewInvMatrix[eye]);

    // Retrieve the particle parameters.
    retval.SphereParams = Particles[particleID].Position;
    const float3 pos = retval.SphereParams.xyz;
    const float rad = retval.SphereParams.w;

#ifdef FLOAT_COLOUR
    retval.Colour = Particles[particleID].Colour;
#else /* FLOAT_COLOUR */
    {
        uint colour = Particles[particleID].Colour;
        retval.Colour.r = (colour & 0xFF) / 255.0f;
        colour >>= 8;
        retval.Colour.g = (colour & 0xFF) / 255.0f;
        colour >>= 8;
        retval.Colour.b = (colour & 0xFF) / 255.0f;
        colour >>= 8;
        retval.Colour.a = colour / 255.0f;
    }
#endif /* FLOAT_COLOUR */

    retval.Position.xyz *= rad;         // Scale to sphere size.

                                        // Orient the sprite towards the camera.
    float3 v = normalize(retval.SphereParams.xyz - retval.CameraPosition.xyz);
    float3 u = normalize(ViewInvMatrix[eye]._21_22_23);// retval.CameraDirection.xyz;//
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));
    float4x4 matOrient = float4x4(
        float4(r, 0.0f),
        float4(u, 0.0f),
        float4(v, 0.0f),
        float4(0.0f.xxx, 1.0f));
    retval.Position = mul(retval.Position, matOrient);

    // Move sprite to world position.
    retval.Position.xyz += pos;
    retval.Position -= float4(rad * v, 0.0f);

    // Do the camera transform.
    retval.Position = mul(retval.Position, mvp);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos;
#ifdef HOLOMOL
    retval.Eye = eye;
#endif /* HOLOMOL */

    return retval;
}
#endif
