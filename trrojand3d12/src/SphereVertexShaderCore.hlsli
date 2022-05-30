// <copyright file="SphereVertexShaderCore.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "ColourConversion.hlsli"
#include "LocalLighting.hlsli"
#include "OrientToCamera.hlsli"
#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"
#include "TransferFunction.hlsli"


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


#if defined(INSTANCING)
/// <summary>
/// The buffer holding the particle parameters.
/// </summary>
StructuredBuffer<Particle> Particles : register(t1);
#endif /* defined(INSTANCING) */


#if defined(SPHERE_INST)
#define VsInput VsGeometryInput
#elif (defined(INSTANCING))
#define VsInput VsNoInput
#else /* defined(SPHERE_INST) */
#define VsInput VsRaycastingInput
#endif /* defined(SPHERE_INST) */


#if defined(SPHERE_INST)
#define VsOutput PsGeometryInput
#elif defined(INSTANCING)
#define VsOutput PsRaycastingInput
#else /* defined(SPHERE_INST) */
#define VsOutput VsPassThroughOutput
#endif /* defined(SPHERE_INST) */


/// <summary>
/// Vertex shader for instancing a sprite from nothing or a sphere from the
/// vertex buffer.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
[RootSignature(SphereRootSignature)]
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
    const float3 pos = Particles[particleID].SphereParams.xyz;

#if defined(PER_VERTEX_RADIUS)
    const float rad = Particles[particleID].SphereParams.w;
#else /* defined(PER_VERTEX_RADIUS)*/
    const float rad = GlobalRadius;
#endif /* defined(PER_VERTEX_RADIUS)*/

#if (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY))
    float intensity = Particles[particleID].Intensity;
#elif (defined(PER_VERTEX_COLOUR) && defined(FLOAT_COLOUR))
    float4 colour = Particles[particleID].Colour;
#elif defined(PER_VERTEX_COLOUR)
    float4 colour = UintToFloat4Colour(Particles[particleID].Colour);
#else /* (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)) */
    float4 colour = GlobalColour;
#endif /* (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)) */

#else /* defined(INSTANCING) */
    /* Select the sphere parameters from vertex buffer. */
    const float3 pos = input.SphereParams.xyz;

#if defined(PER_VERTEX_RADIUS)
    const float rad = input.SphereParams.w;
#else /* defined(PER_VERTEX_RADIUS)*/
    const float rad = GlobalRadius;
#endif /* defined(PER_VERTEX_RADIUS)*/

#if defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)
    float intensity = input.Intensity;
#elif defined(PER_VERTEX_COLOUR)
    float4 colour = input.Colour;
#else  /* (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)) */
    float4 colour = GlobalColour;
#endif  /* (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY)) */
#endif /* defined(INSTANCING) */

    /* Generate vertex from nothing based on the vertex ID. */
#if defined(QUAD_INST)
    const float2 FLIP_Y = float2(1.0f, -1.0f);  // Flips the y-axis of a float2.
    //Note: this will flip faces: retval.Position = float4(input.VertexID / 2, input.VertexID % 2, 0.0f, 1.0f);
    retval.Position = float4(input.VertexID % 2, input.VertexID / 2, 0.0f, 1.0f);
    retval.Position.xy = 2.0f * FLIP_Y * (retval.Position.xy - 0.5f.xx);
    retval.Position.xyz *= rad;

#elif defined(POLY_INST)
#error "POLY_INST is not yet implemented: Compute polar coordinates from input.VertexID here."
#endif /* defined(QUAD_INST) */

#if defined(SPHERE_INST)
    /* Transform sphere geometry from VB for vertex shader stage. */
    retval.Position = float4(input.Position, 1.0f);
    retval.Position.xyz *= rad;
    retval.Position.xyz += pos.xyz;
    retval.Position = mul(retval.Position, mvp);

    // Retrieve data for later shading.
    retval.WorldPosition = input.Position.xyz;
    retval.WorldNormal = input.Normal;
    retval.ViewDirection = normalize(invVm._31_32_33);

#elif defined(INSTANCING)
    /*
     * Transform sprite for per-pixel raycasting techniques working directly on
     * instanced sprites.
     */

    // Reconstruct the camera system.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, invVm);

    // Orient the sprite towards the camera.
    float4x4 matOrient = OrientToCamera(pos, invVm);
    retval.Position = mul(retval.Position, matOrient);

    // Move sprite to world position.
    retval.Position.xyz += pos;
    retval.Position.xyz -= rad * matOrient._31_32_33;

    // Do the camera transform.
    retval.Position = mul(retval.Position, mvp);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos;

    // Pass on world-space parameters for raycasting.
    retval.SphereParams = float4(pos, rad);

#if defined(PER_VERTEY_RAY)
    retval.Ray = normalize(pos - retval.CameraPosition.xyz);
#endif /* defined(PER_VERTEY_RAY) */

#else /* defined(SPHERE_INST) */
    /* Pass through data to geometry or hull shader. */
    retval.SphereParams = float4(pos, rad);
#endif /* defined(SPHERE_INST) */

#if defined(PER_VERTEX_INTENSITY)
    /* Per-vertex intensity is transformed to colour in vertex shader.*/
    float texCoords = TexCoordsFromIntensity(intensity, IntensityRange);
    retval.Colour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
#elif defined(PER_PIXEL_INTENSITY)
    /* Per-vertex intensity is passed trough to pixel shader. */
    retval.Intensity = intensity;
#else /* defined(PER_VERTEX_INTENSITY) */
    /* Pass through vertex colour.*/
    retval.Colour = colour;
#endif /* defined(PER_VERTEX_INTENSITY) */

#if defined(HOLOMOL)
    retval.Eye = eye;
#endif /* defined(HOLOMOL) */

    return retval;
}
