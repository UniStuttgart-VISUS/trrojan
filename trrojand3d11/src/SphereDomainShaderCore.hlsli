/// <copyright file="SphereDomainShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "LocalLighting.hlsli"
#include "OrientToCamera.hlsli"
#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"


// The number of control points, which is one per splat.
#define CNT_CONTROL_POINTS (1)


#define VsOutput VsPassThroughOutput


#if (defined(QUAD_TESS) || defined(POLY_TESS) || defined(ADAPT_POLY_TESS))
#define PsInput PsRaycastingInput
#elif (defined(SPHERE_TESS) || defined(ADAPT_SPHERE_TESS) || defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS))
#define PsInput PsGeometryInput
#endif /* (defined(QUAD_TESS) || defined(POLY_TESS) || defined(ADAPT_POLY_TESS)) */


/// <summary>
/// Domain for computing the vertex positions of a sprite, hemisphere or
/// sphere.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
[domain("quad")]
PsInput Main(OutputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        HsConstants constants,
        float2 uv : SV_DomainLocation) {
    PsInput retval = (PsInput) 0;

    const float PI = 3.14159265358979323846f;
    const float TWO_PI = 2.0f * PI;
#if defined(HOLOMOL)
    const uint eye = patch[0].Eye;
#else /* defined(HOLOMOL) */
    const uint eye = 0;
#endif /* defined(HOLOMOL) */

    // Select the right matrices.
    float4x4 mvp = ViewProjMatrix[eye];
    float4x4 pm = ProjMatrix[eye];
    float4x4 vm = ViewMatrix[eye];
    float4x4 vmInv = ViewInvMatrix[eye];

    // Get the world-space centre of the sphere.
    const float3 pos = patch[0].SphereParams.xyz;
    float rad = patch[0].SphereParams.w;

#if defined(RAYCASTING)
    /* Move vertices to locations on sprite. */

    // Reconstruct the camera system for the pixel shader.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, vmInv);

    // Pass through sphere parameters and colour.
    retval.SphereParams = patch[0].SphereParams;

#if defined(QUAD_TESS)
    float4 coords = float4(uv.xy, 0.0f, 1.0f);
    coords.xy -= 0.5f.xx;
    coords.xy *= rad;

#else /* defined(QUAD_TESS) */
    // If we use the radius of the sphere as size of the triangle fan, its hull
    // are the secants of the final sphere, but we need to have the tangent.
    // Adjust the value such that the radius is equal to the altitude (apothem)
    // of the triangle.
    float alpha = TWO_PI / (2.0f * constants.EdgeTessFactor[0]);
    rad /= cos(alpha);
    //rad /= 0.9999398715340;

    // Compute polar coordinates for the fan around the sphere.
    float phi = TWO_PI * uv.x;
    float sinPhi, cosPhi;
    sincos(phi, sinPhi, cosPhi);
    float4 coords = float4(rad * cosPhi, rad * sinPhi, 0.0f, 1.0f);

#endif /* defined(QUAD_TESS) */

    // Orient the sprite towards the camera.
    float4x4 matOrient = OrientToCamera(pos, vmInv);
    coords = mul(retval.Position, matOrient);

    // Move sprite to world position.
    coords.xyz += pos;
    coords.xyz -= rad * matOrient._31_32_33;

    // Perform projection.
    retval.Position = mul(coords, mvp);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos.xyz;

#else
    /* 3D sphere tessellation: Move vertices to locations on sphere. */

    // Reconstruct the view direction for shading.
    float4 camPos, up, right;
    ReconstructCamera(camPos, retval.ViewDirection, up, right, vmInv);

    // Move vertices of the patch to a sphere.
    float phi = PI * uv.x;
#if (defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS))
    float theta = PI * uv.y;
#else /* (defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS)) */
    float theta = TWO_PI * uv.y;
#endif /* (defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS)) */

    float sinPhi, cosPhi, sinTheta, cosTheta;
    sincos(phi, sinPhi, cosPhi);
    sincos(theta, sinTheta, cosTheta);

    float4 coords = float4(
        rad * sinPhi * cosTheta,
        rad * sinPhi * sinTheta,
        rad * cosPhi,
        1.0f);

#if (defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS))
    // Orient the hemisphere towards the camera.
    float3 v = normalize(pos - camPos.xyz);
    float3 u = normalize(vmInv._21_22_23);
    float3 r = normalize(cross(v, u));
    u = normalize(cross(r, v));
    float4x4 matOrient = float4x4(
        float4(r, 0.0f),
        -float4(v, 0.0f),
        float4(u, 0.0f),
        float4(0.0f.xxx, 1.0f));
    //float4x4 matOrient = OrientToCamera(pos, invVm);
    //float4 v = matOrient._31_32_33_34;
    //matOrient._31_32_33_34 = matOrient._21_22_23_24;
    //matOrient._21_22_23_24 = -v;
    coords = mul(coords, matOrient);
#endif /* (defined(HEMISPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS)) */

    // The normal are the sphere coordinates.
    retval.WorldNormal = normalize(coords.xyz);

    // Move the sphere to the right location.
    coords.xyz += pos;
    coords = mul(coords, vm);
    //retval.WorldPosition = coords.xyz;

    // Project the vertices.
    retval.Position = mul(coords, pm);
#endif /* defined(RAYCASTING) */

#if defined(PER_PIXEL_INTENSITY)
    retval.Intensity = patch[0].Intensity;
#else /* defined(PER_PIXEL_INTENSITY) */
    retval.Colour = patch[0].Colour;
#endif /* defined(PER_PIXEL_INTENSITY) */

#if defined(HOLOMOL)
    retval.Eye = eye;
#endif /* defined(HOLOMOL) */

    return retval;
}
