/// <copyright file="SphereGeometryShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"



void ReconstructCamera(out float4 pos, out float4 dir, out float4 up,
        out float4 right, const in matrix viewInvMatrix) {
    // calculate cam position
    pos = viewInvMatrix._41_42_43_44; // (C) by Christoph

    dir = float4(normalize(viewInvMatrix._31_32_33_34.xyz), 0.0);
    up = normalize(viewInvMatrix._21_22_23_24);
    right = float4(normalize(cross(dir.xyz, up.xyz)), 0.0);

    up = float4(normalize(cross(right.xyz, dir.xyz)), 0.0);
}


/// <summary>
/// Geometry shader creating a sprite from a single vertex.
/// </summary>
[maxvertexcount(4)]
void Main(point GsInput input[1], inout TriangleStream<PsInput> triStream) {
    PsInput v = (PsInput)0;

    //// Take a sample directly in the middle of the pixel at 0, 0, which is 0 / Width + 1 / (Width * 2)
    //// Note: We changed NVIDIA's linear sampler to a point sampler, so we can
    //// just take the pixel at (0, 0).
    //float3 stereoParms = TexStereoParams.SampleLevel(SamplerStereoParams, 0.0.xx, 0); //float2(0.0625, 0.5));
    //v.EyeSeparation = stereoParms.x;
    //v.Convergence = stereoParms.y;

    //uint eye = input[0].Eye;
    float4x4 mvp = ViewProjMatrix;
    float rad = input[0].Radius;

    //#define MAJOR_DOWELING_RADIUS
#ifdef MAJOR_DOWELING_RADIUS
    rad *= 5;
#endif
    float squareRad = rad * rad;

    float4 objPos = input[0].Position;
    objPos.w = 1.0;
    v.Colour = input[0].Colour;
    v.SphereParams = float4(input[0].Position.xyz, rad);
    //v.Eye = eye;

    // Reconstruct camera system.
    ReconstructCamera(v.CameraPosition, v.CameraDirection, v.CameraUp,
        v.CameraRight, ViewInvMatrix);

    // Transform camera to glyph space and undo stereo transform.
    v.CameraPosition.xyz -= objPos.xyz;
    //have separate matrices on hololens to replace the following:
    //v.CameraPosition.xyz += v.CameraRight * v.EyeSeparation;

    // SphereParams-Touch-Plane-Approach™
    float2 winHalf = 2.0 / Viewport.zw; // window size
    float2 d, p, q, h, dd;
    float2 mins, maxs;
    float3 testPos;
    float4 projPos;

#ifdef HALO
    squarRad = (rad + HALO_RAD) * (rad + HALO_RAD);
#endif // HALO

#if 0
#define DUEBEL 1.5f
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
#else

    // projected camera vector
    float3 c2 = float3(dot(v.CameraPosition.xyz, v.CameraRight),
        dot(v.CameraPosition.xyz, v.CameraUp),
        dot(v.CameraPosition.xyz, v.CameraDirection));

    float3 cpj1 = v.CameraDirection * c2.z + v.CameraRight * c2.x;
    float3 cpm1 = v.CameraDirection * c2.x - v.CameraRight * c2.z;

    float3 cpj2 = v.CameraDirection * c2.z + v.CameraUp * c2.y;
    float3 cpm2 = v.CameraDirection * c2.y - v.CameraUp * c2.z;

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
#endif
}
