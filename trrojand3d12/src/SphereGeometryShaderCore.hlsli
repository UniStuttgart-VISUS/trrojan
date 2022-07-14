// <copyright file="SphereGeometryShaderCore.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the 3-clause BSD licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "OrientToCamera.hlsli"
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
[RootSignature(SHADER_ROOT_SIGNATURE)]
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
    float4x4 invVm = ViewInvMatrix[eye];
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
        v.CameraRight, invVm);

    // Transform camera to glyph space and undo stereo transform.
    v.CameraPosition.xyz -= objPos.xyz;
    //have separate matrices on hololens to replace the following:
    //v.CameraPosition.xyz += v.CameraRight * v.EyeSeparation;

#if defined(STPA)
    // Sphere-Touch-Plane-Approach�
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
    //if (!(any(d < p)
    //        || (length(v.CameraPosition.xyz) < rad)
    //        || dot(v.CameraDirection.xyz, v.CameraPosition.xyz) < 0)) {
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
        v.Position = float4(mins.x, mins.y, projPos.z, 1.0f);
#if defined(PER_VERTEX_RAY)
        v.Ray = normalize(v.Position.xyz - v.CameraPosition.xyz);
#endif /* defined(PER_VERTEX_RAY) */
        triStream.Append(v);

        //top left
        v.Position = float4(mins.x, maxs.y, projPos.z, 1.0f);
#if defined(PER_VERTEX_RAY)
        v.Ray = normalize(v.Position.xyz - v.CameraPosition.xyz);
#endif /* defined(PER_VERTEX_RAY) */
        triStream.Append(v);

        //bottom right
        v.Position = float4(maxs.x, mins.y, projPos.z, 1.0f);
#if defined(PER_VERTEX_RAY)
        v.Ray = normalize(v.Position.xyz - v.CameraPosition.xyz);
#endif /* defined(PER_VERTEX_RAY) */
        triStream.Append(v);

        //top right
        v.Position = float4(maxs.x, maxs.y, projPos.z, 1.0f);
#if defined(PER_VERTEX_RAY)
        v.Ray = normalize(v.Position.xyz - v.CameraPosition.xyz);
#endif /* defined(PER_VERTEX_RAY) */
        triStream.Append(v);

        triStream.RestartStrip();
    //}
#endif /* defined(DUEBEL) */

#elif defined(GEO_QUAD)
    const float4 CORNERS[] = {
        float4(-1.0f, -1.0f, 0.0f, 1.0f),
        float4(-1.0f, 1.0f, 0.0f, 1.0f),
        float4(1.0f, -1.0f, 0.0f, 1.0f),
        float4(1.0f, 1.0f, 0.0f, 1.0f),
    };
    float4x4 matOrient = OrientToCamera(objPos.xyz, invVm);

    [unroll(4)]
    for (int i = 0; i < 4; ++i) {
        v.Position = float4(CORNERS[i]);
        v.Position.xyz *= rad;

        // Orient the sprite towards the camera.
        v.Position = mul(v.Position, matOrient);

        // Move sprite to world position.
        v.Position.xyz += objPos.xyz;
        v.Position.xyz -= rad * matOrient._31_32_33;

        // Compute ray in object space.
#if defined(PER_VERTEX_RAY)
        v.Ray = normalize(v.Position.xyz - v.CameraPosition.xyz);
#endif /* defined(PER_VERTEX_RAY) */

        // Do the camera transform.
        v.Position = mul(v.Position, mvp);

        triStream.Append(v);
    }

    triStream.RestartStrip();

#elif defined(GEO_POLY)
    const float PI = 3.14159265358979323846f;
    const float TWO_PI = 2.0f * PI;
    float4x4 matOrient = OrientToCamera(objPos.xyz, invVm);

    // Compute apothem of triangle fan enclosing the whole sphere.
    uint cnt = min(PolygonCorners, CNT_MAX_VERTICES);
    float alpha = TWO_PI / (2.0f * cnt);
    rad /= cos(alpha);

    float phi = 0.0f;
    float sinPhi = 0.0f;
    float cosPhi = 1.0f;

    v.Position = float4(rad * cosPhi, rad * sinPhi, 0.0f, 1.0f);

    // Orient the sprite towards the camera.
    v.Position = mul(v.Position, matOrient);

    // Move sprite to world position.
    v.Position.xyz += objPos.xyz;
    v.Position.xyz -= rad * matOrient._31_32_33;

    // Do the camera transform.
    v.Position = mul(v.Position, mvp);

    triStream.Append(v);

    // https://www.gamedev.net/forums/topic/199741-convex-polygon-to-triangle-strip/
    uint i = 1;
    uint j = cnt - 1;
    bool k = false;

    while (i < j) {
        if (k) {
            phi = i++ / cnt;
        } else {
            phi = TWO_PI - (j-- / cnt);
        }

        sincos(phi, sinPhi, cosPhi);
        v.Position = float4(rad * cosPhi, rad * sinPhi, 0.0f, 1.0f);

        // Orient the sprite towards the camera.
        v.Position = mul(v.Position, matOrient);

        // Move sprite to world position.
        v.Position.xyz += objPos.xyz;
        v.Position.xyz -= rad * matOrient._31_32_33;

        // Do the camera transform.
        v.Position = mul(v.Position, mvp);

        triStream.Append(v);

        k = !k;
    }

    triStream.RestartStrip();

#else /* defined(STPA) */
#error "Unsupported geometry shader technique."
#endif /* defined(STPA) */
}
