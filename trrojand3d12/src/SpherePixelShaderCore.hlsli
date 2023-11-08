// <copyright file="SpherePixelShaderCore.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "LocalLighting.hlsli"
#include "SpherePipeline.hlsli"
#include "TransferFunction.hlsli"


#if defined(PER_PIXEL_INTENSITY)
/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);

/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);
#endif /* defined(PER_PIXEL_INTENSITY) */

#if defined(RAYCASTING)
#define PsInput PsRaycastingInput
#else /* defined(RAYCASTING) */
#define PsInput PsGeometryInput
#endif /* defined(RAYCASTING) */


/// <summary>
/// Entry point of the pixel shader doing the raycasting of a sphere on the
/// sprite.
/// </summary>
[RootSignature(SHADER_ROOT_SIGNATURE)]
PsOutput Main(PsInput input/*, bool isFront : SV_IsFrontFace*/) {
    PsOutput retval = (PsOutput) 0;

//#define BILLBOARD
#ifdef BILLBOARD
    //if (isFront) {
    //    retval.Colour = float4(1.0, 0.0, 0.0, 1.0);
    //} else {
    //    retval.Colour = float4(0.0, 6.0, 0.0, 1.0);
    //}
    retval.Colour = float4(1.0, 1.0f, 0.0f, 1.0f);

#if defined(PER_PIXEL_INTENSITY)
    retval.Colour = TransferFunction.SampleLevel(LinearSampler,
        TexCoordsFromIntensity(input.Intensity, IntensityRange), 0);
#else /* defined(PER_PIXEL_INTENSITY) */
    retval.Colour = input.Colour;
#endif /* defined(PER_PIXEL_INTENSITY) */

    return retval;
#endif

#if defined(HOLOMOL)
    const uint eye = input.Eye;
#else /* defined(HOLOMOL) */
    const uint eye = 0;
#endif /* defined(HOLOMOL) */


#if defined(RAYCASTING)
    float4 coord;
    float3 ray;
    float lambda;
    float3 sphereintersection = 0.0.xxx;
    float3 normal;

    float4 camPos = input.CameraPosition;
    float4 camIn = input.CameraDirection;
    float4 camUp = input.CameraUp;
    float4 camRight = input.CameraRight;
    //float eyeSep = input.EyeSeparation;

    float4 objPos = float4(input.SphereParams.xyz, 1.0);

    float4 lightDir = normalize(input.CameraDirection);
    float rad = input.SphereParams.w;
    float squarRad = rad * rad;

#if defined(PER_VERTEX_RAY)
    ray = input.Ray;
#else /* defined(PER_VERTEX_RAY) */
    // Transform fragment coordinates from window coordinates to view
    // coordinates.
    input.Position.y = Viewport.w - input.Position.y;
    coord = input.Position
        * float4(2.0 / Viewport.z, 2.0 / Viewport.w, 1.0, 0.0)
        + float4(-1.0, -1.0, 0.0, 1.0);

    // Transform fragment coordinates from view coordinates to object
    // coordinates...
    coord = mul(ViewProjInvMatrix[eye], coord);
    coord /= coord.w;
    // ... and to glyph space.
    coord -= objPos;

    // Compute the viewing ray.
    ray = coord.xyz - camPos.xyz;
#endif /* defined(PER_VERTEX_RAY) */
    ray = normalize(ray);

//#define DEBUG_RAY
#if defined(DEBUG_RAY)
    retval.Colour = float4(ray, 1.0f);
    return retval;
#endif /* defined(DEBUG_RAY) */

    // calculate the geometry-ray-intersection
    float d1 = -dot(camPos.xyz, ray);                       // projected length of the cam-SphereParams-vector onto the ray
    float d2s = dot(camPos.xyz, camPos.xyz) - d1 * d1;      // off axis of cam-SphereParams-vector and ray
    float radicand = squarRad - d2s;                        // square of difference of projected length and lambda
    lambda = d1 - sqrt(radicand);                           // lambda

    if ((radicand < 0.0f) || (lambda < 0.0f)) {
//#define FILL_BILLBOARD
#if defined(FILL_BILLBOARD)
        retval.Colour = 0.8.xxxx;
#else /* defined(FILL_BILLBOARD) */
        discard;
#endif /* defined(FILL_BILLBOARD) */

    } else {
        // chose color for lighting
        sphereintersection = lambda * ray + camPos.xyz;    // intersection point
                                                           // "calc" normal at intersection point
        normal = sphereintersection / rad;

#if defined(PER_PIXEL_INTENSITY)
        // The colour is the intensity value, which for we need to perform a
        // texture lookup before shading.
        float texCoords = TexCoordsFromIntensity(input.Intensity, IntensityRange);
        float4 baseColour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
        //baseColour = float4(1.0f, 0.0f, 0.0f, 1.0f);
#else /* defined(PER_PIXEL_INTENSITY) */
        // Colour is explicitly given or transfer function has already been
        // sampled in vertex shader stage.
        float4 baseColour = input.Colour;
        //baseColour = float4(0.0f, 1.0f, 0.0f, 1.0f);
#endif /* defined(PER_PIXEL_INTENSITY) */

        retval.Colour = float4(LocalLighting(ray, normal, lightDir.xyz, baseColour.rgb), baseColour.a);
        //retval.Colour = baseColour;
    }

    // calculate depth
#define DEPTH
#ifdef DEPTH
    // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
    float4 intPos = float4(sphereintersection + objPos.xyz, 1.0);
    float dz = dot(ViewProjMatrix[eye]._31_32_33_34, intPos);
    float dw = dot(ViewProjMatrix[eye]._41_42_43_44, intPos);
    retval.Depth = (dz/dw);
    
    //float4 proj_space = mul(ViewProjMatrix[eye], intPos);
    //retval.Depth = (proj_space.z/proj_space.w);
#else
    retval.Depth = input.Position.z;
#endif // DEPTH

#else /* defined(RAYCASTING) */
    /* Shade geometry based on surface position and normal. */

#if defined(PER_PIXEL_INTENSITY)
    // The colour is the intensity value, which for we need to perform a
    // texture lookup before shading.
    float texCoords = TexCoordsFromIntensity(input.Intensity, IntensityRange);
    //texCoords = 1.0f - texCoords;
    float4 baseColour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
#else /* defined(PER_PIXEL_INTENSITY) */
    // Colour is explicitly given or transfer function has already been
    // sampled in vertex shader stage.
    float4 baseColour = input.Colour;
#endif /* defined(PER_PIXEL_INTENSITY) */

    float3 ray = input.ViewDirection.xyz;
    retval.Colour = float4(LocalLighting(-ray, normalize(input.WorldNormal.xyz),
        ray, baseColour.rgb), baseColour.a);
    //retval.Colour = float4(input.WorldNormal.xyz, 1.0f);
    //retval.Colour = float4(-ray, 1.0f);
#endif /* defined(RAYCASTING) */

    //retval.Colour = float4(1.0, 0.0, 0.0, 1.0);
    return retval;
}
