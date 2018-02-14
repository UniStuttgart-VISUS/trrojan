/// <copyright file="SphereVertexShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "ColourConversion.hlsli"
#include "LocalLighting.hlsli"
#include "OrientToCamera.hlsli"
#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"
#include "TransferFunction.hlsli"

#if (defined(QUAD_INST) || defined(POLY_INST) || defined(SPHERE_INST))
#define INSTANCING (1)
#endif /* (defined(QUAD_INST) || defined(POLY_INST) || defined(SPHERE_INST)) */


#if defined(PER_VERTEX_INTENSITY)
/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);

/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);
#endif /* defined(PER_VERTEX_INTENSITY) */


/// <summary>
/// The buffer holding the particle parameters.
/// </summary>
StructuredBuffer<Particle> Particles : register(t1);


#if (defined(QUAD_INSTANCING) || defined(POLY_INSTANCING))
#define VsInput VsNoInput
#define VsOutput PsRaycastingInput
#elif defined(GEOMETRY_INSTANCING)
#define VsInput VsGeometryInput
#define VsOutput PsGeometryInput
#endif /* (defined(QUAD_INSTANCING) || defined(POLY_INSTANCING)) */

// #define VsInput VsRaycastingInput



/// <summary>
/// Vertex shader for instancing a sprite from nothing or a sphere from the
/// vertex buffer.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

#if defined(INSTANCING)
    /* Determine active index in structured resource view. */
    uint particleID = input.InstanceID;
#if defined(HOLOMOL)
    uint eye = particleID % 2;
    particleID /= 2;
#else
    uint eye = 0;
#endif

#elif defined(HOLOMOL)
    /* For non-instanced geometry, use the eye directly. */
    uint eye = input.Eye;
#else /* defined(INSTANCING) */
    uint eye = 0;
#endif /* defined(INSTANCING) */

    /* Select the right matrices. */
    float4x4 mvp = ViewProjMatrix[eye];
    float4x4 invVm = ViewInvMatrix[eye];

#if defined(INSTANCING)
    /* Select the sphere parameters from the structured buffer. */
    const float3 pos = Particles[particleID].Position.xyz;

#if defined(PER_VERTEX_RADIUS)
    const float rad = Particles[particleID].Position.w;
#else
    const float rad = GlobalRadius;
#endif

#if (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY))
    float intensity = Particles[particleID].Intensity;
#elif (defined(PER_VERTEX_COLOUR) && defined(FLOAT_COLOUR))
    float4 colour = Particles[particleID].Colour;
#elif defined(PER_VERTEX_COLOUR)
    float4 colour = UintToFloat4Colour(Particles[particleID].Colour);
#else
    float4 colour = GlobalColour;
#endif

#else /* defined(INSTANCING) */
    /* Select the sphere parameters from vertex buffer. */
    const float3 pos = input.Position.xyz;

#if defined(PER_VERTEX_RADIUS)
    const float rad = input.Position.w;
#else
    const float rad = GlobalRadius;
#endif

#if defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)
    float intensity = input.Intensity;
#elif defined(PER_VERTEX_COLOUR)
    float4 colour = input.Colour;
#else
    float4 colour = GlobalColour;
#endif
#endif /* defined(INSTANCING) */

#if defined(QUAD_INST)
    /* Make the right vertex based on the vertex ID. */
    const float2 FLIP_Y = float2(1.0f, -1.0f);  // Flips the y-axis of a float2.
    retval.Position = float4(input.VertexID % 2, input.VertexID % 4 / 2,
        0.0f, 1.0f);
    retval.Position.xy = 2.0f * FLIP_Y * (pos.xy - 0.5f.xx);
    retval.Position.xyz *= rad;

#elif defined(POLY_INST)
#error "POLY_INST is not yet implemented: Compute polar coordinates from input.VertexID here."
#endif /* defined(QUAD_INST) */

#if 0
    //"QUAD_TESS"
    //"POLY_TESS"
    //"ADAPT_POLY_TESS"
    //"SPTA"
    //"GEO_QUAD"
    //"GEO_POLY"
    //"SPHERE_TESS"
    //"ADAPT_SPHERE_TESS"
    //"HEMISPHERE_TESS"
    //"ADAPT_HEMISPHERE_TESS"



    // Reconstruct the camera system.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, invVm);

    // Orient the sprite towards the camera.
    float4x4 matOrient = OrientToCamera(pos, invVm);
    retval.Position = mul(retval.Position, matOrient);

    // Move sprite to world position.
    retval.Position.xyz += pos;
    retval.Position -= float4(rad * matOrient._31_32_33, 0.0f);

    // Do the camera transform.
    retval.Position = mul(retval.Position, mvp);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos;

    // Pass on world-space parameters for raycasting.
    retval.SphereParams = float4(pos, rad);

#elif defined(GEOMETRY_INSTANCING)
    // Transform the instance to match the sphere's size and position.
    retval.Position = float4(input.Position, 1.0f);
    retval.Position.xyz *= rad;
    retval.Position.xyz += pos.xyz;
    retval.Position = mul(retval.Position, mvp);

    // Retrieve the view direction for later shading.
    retval.ViewDirection = float4(normalize(invVm._31_32_33), 0.0);
#endif /* defined(QUAD_INSTANCING) */

#if defined(PER_VERTEX_INTENSITY)
    /* Per-vertex intensity is transformed to colour in vertex shader.*/
    float texCoords = TexCoordsFromIntensity(intensity, IntensityRange);
    retval.Colour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
#elif defined(PER_PIXEL_INTENSITY)
    /* Per-vertex intensity is passed trough to pixel shader. */
    retval.Colour = intensity.rrrr;
#else /* defined(PER_VERTEX_INTENSITY) */
    /* Pass through vertex colour.*/
    retval.Colour = colour;
#endif /* defined(PER_VERTEX_INTENSITY) */

#if defined(HOLOMOL)
    retval.Eye = eye;
#endif /* defined(HOLOMOL) */

    return retval;
}
