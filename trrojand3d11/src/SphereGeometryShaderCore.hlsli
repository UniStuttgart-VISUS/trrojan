/// <copyright file="SphereGeometryShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"


#if (defined(STPA) || defined(GEO_QUAD))
#define CNT_MAX_VERTICES (4)
#else /* (defined(STPA) || defined(GEO_QUAD)) */
#define CNT_MAX_VERTICES (32)
#endif /* (defined(STPA) || defined(GEO_QUAD)) */


#define VsOutput VsPassThroughOutput
#define PsInput PsRaycastingInput


/// <summary>
/// Geometry shader creating a sprite from a single vertex.
/// </summary>
[maxvertexcount(CNT_MAX_VERTICES)]
void Main(point VsOutput input[1], inout TriangleStream<PsInput> triStream) {
    PsInput v = (PsInput) 0;

    //// Take a sample directly in the middle of the pixel at 0, 0, which is 0 / Width + 1 / (Width * 2)
    //// Note: We changed NVIDIA's linear sampler to a point sampler, so we can
    //// just take the pixel at (0, 0).
    //float3 stereoParms = TexStereoParams.SampleLevel(SamplerStereoParams, 0.0.xx, 0); //float2(0.0625, 0.5));
    //v.EyeSeparation = stereoParms.x;
    //v.Convergence = stereoParms.y;

#ifdef HOLOMOL
    const uint eye = input[0].Eye;
#else /* HOLOMOL */
    const uint eye = 0;
#endif /* HOLOMOL */
    float4x4 mvp = ViewProjMatrix[eye];
    float rad = input[0].SphereParams.w;

    //#define MAJOR_DOWELING_RADIUS
#ifdef MAJOR_DOWELING_RADIUS
    rad *= 5;
#endif /* MAJOR_DOWELING_RADIUS */
    float squareRad = rad * rad;

    float4 objPos = float4(input[0].SphereParams.xyz, 1.0f);
#if defined(PER_PIXEL_INTENSITY)
    v.Intensity = input[0].Intensity;
#else /* defined(PER_PIXEL_INTENSITY) */
    v.Colour = input[0].Colour;
#endif /* defined(PER_PIXEL_INTENSITY) */
    v.SphereParams = input[0].SphereParams;
#if defined(HOLOMOL)
    v.Eye = eye;
#endif /* defined(HOLOMOL) */

    // Reconstruct camera system.
    ReconstructCamera(v.CameraPosition, v.CameraDirection, v.CameraUp,
        v.CameraRight, ViewInvMatrix[eye]);

    // Transform camera to glyph space and undo stereo transform.
    v.CameraPosition.xyz -= objPos.xyz;
    //have separate matrices on hololens to replace the following:
    //v.CameraPosition.xyz += v.CameraRight * v.EyeSeparation;

#if defined(STPA)
    // Sphere-Touch-Plane-Approach™
    //float2 winHalf = 2.0 / Viewport.zw; // window size
    float2 d, p, q, h, dd;
    float2 mins, maxs;
    float3 testPos;
    float4 projPos;

//#define DUEBEL 1.2f
#if defined(DUEBEL)
    //bottom left
    v.Position = mul(objPos - v.CameraUp * DUEBEL * rad - v.CameraRight * DUEBEL * rad, mvp);
    triStream.Append(v);

    //bottom right
    v.Position = mul(objPos - v.CameraUp * DUEBEL * rad + v.CameraRight * DUEBEL * rad, mvp);
    triStream.Append(v);

    //top left
    v.Position = mul(objPos + v.CameraUp * DUEBEL * rad - v.CameraRight * DUEBEL * rad, mvp);
    triStream.Append(v);

    //top right
    v.Position = mul(objPos + v.CameraUp * DUEBEL * rad + v.CameraRight * DUEBEL * rad, mvp);
    triStream.Append(v);

    triStream.RestartStrip();
#else /*  defined(DUEBEL) */
    // projected camera vector
    float3 c2 = float3(dot(v.CameraPosition.xyz, v.CameraRight.xyz),
        dot(v.CameraPosition.xyz, v.CameraUp.xyz),
        dot(v.CameraPosition.xyz, v.CameraDirection.xyz));

    float3 cpj1 = v.CameraDirection.xyz * c2.z + v.CameraRight.xyz * c2.x;
    float3 cpm1 = v.CameraDirection.xyz * c2.x - v.CameraRight.xyz * c2.z;

    float3 cpj2 = v.CameraDirection.xyz * c2.z + v.CameraUp.xyz * c2.y;
    float3 cpm2 = v.CameraDirection.xyz * c2.y - v.CameraUp.xyz * c2.z;

    d.x = length(cpj1);
    d.y = length(cpj2);

    dd = 1.0.xx / d;

    p = squareRad * dd;
    q = d - p;
    h = sqrt(p * q);
    //h = float2(0.0);

    p *= dd;
    h *= dd;

    cpj1 *= p.x;
    cpm1 *= h.x;
    cpj2 *= p.y;
    cpm2 *= h.y;

    testPos = objPos.xyz + cpj1 + cpm1;
    projPos = mul(float4(testPos, 1.0), mvp);
    ///projPos = mul(mvp, float4(testPos, 1.0));
    projPos /= projPos.w;
    mins = projPos.xy;
    maxs = projPos.xy;

    testPos -= 2.0 * cpm1;
    projPos = mul(float4(testPos, 1.0), mvp);
    ///projPos = mul(mvp, float4(testPos, 1.0));
    projPos /= projPos.w;
    mins = min(mins, projPos.xy);
    maxs = max(maxs, projPos.xy);

    testPos = objPos.xyz + cpj2 + cpm2;
    projPos = mul(float4(testPos, 1.0), mvp);
    ///projPos = mul(mvp, float4(testPos, 1.0));
    projPos /= projPos.w;
    mins = min(mins, projPos.xy);
    maxs = max(maxs, projPos.xy);

    testPos -= 2.0 * cpm2;
    projPos = mul(float4(testPos, 1.0), mvp);
    ///projPos = mul(mvp, float4(testPos, 1.0));
    projPos /= projPos.w;
    mins = min(mins, projPos.xy);
    maxs = max(maxs, projPos.xy);

    //bottom left
    v.Position = float4(mins.x, mins.y, projPos.z, 1.0);
    triStream.Append(v);

    //top left
    v.Position = float4(mins.x, maxs.y, projPos.z, 1.0);
    triStream.Append(v);

    //bottom right
    v.Position = float4(maxs.x, mins.y, projPos.z, 1.0);
    triStream.Append(v);

    //top right
    v.Position = float4(maxs.x, maxs.y, projPos.z, 1.0);
    triStream.Append(v);

    triStream.RestartStrip();
#endif /* defined(DUEBEL) */

#elif defined(GEO_QUAD)
#pragma message "TODO GEO_QUAD"
#elif defined(GEO_POLY)
#pragma message "TODO GEO_POLY"
#else /* defined(STPA) */
#error "Unsupported geometry shader technique."
#endif /* defined(STPA) */
}
