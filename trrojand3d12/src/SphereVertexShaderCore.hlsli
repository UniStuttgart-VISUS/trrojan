// <copyright file="SphereVertexShaderCore.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
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
Texture1D TransferFunction : register(t0);

#if defined(INSTANCING)
StructuredBuffer<Particle> Particles : register(t1);
#endif /* defined(INSTANCING) */

#elif defined(INSTANCING)
StructuredBuffer<Particle> Particles : register(t0);
#endif /* defined(PER_VERTEX_INTENSITY) */


#if defined(PER_VERTEX_INTENSITY)
SamplerState LinearSampler : register(s0);
#endif /* defined(PER_VERTEX_INTENSITY) */


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
[RootSignature(SHADER_ROOT_SIGNATURE)]
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

#elif defined(TRI_INST)
    const float3x3 vertex_positions = {
        -1.7321f, -1.0f, 0.0f, // row 1
        0.0f, 2.0f, 0.0f, // row 2
        1.7321f, -1.0f, 0.0f // row 3
    };
    retval.Position = float4(vertex_positions[input.VertexID], 1.0f);
    retval.Position.xyz *= rad;

#elif defined(POLY_INST)
#error "POLY_INST is not yet implemented: Compute polar coordinates from input.VertexID here."
#endif /* defined(QUAD_INST) */

#if defined(SPHERE_INST)
    /* Transform sphere geometry from VB for vertex shader stage. */
    retval.Position = float4(input.Position, 1.0f);
    retval.Position.xyz *= rad;
    retval.Position.xyz += pos.xyz;
    retval.Position = mul(mvp, retval.Position);

    // Retrieve data for later shading.
    retval.WorldPosition = input.Position.xyz;
    retval.WorldNormal = input.Normal;
    retval.ViewDirection = normalize(invVm._13_23_33);

#elif defined(INSTANCING)
    /*
     * Transform sprite for per-pixel raycasting techniques working directly on
     * instanced sprites.
     */

    // Reconstruct the camera system.
    ReconstructCamera(retval.CameraPosition, retval.CameraDirection,
        retval.CameraUp, retval.CameraRight, invVm);

    //// Account for contact point of rays being potentially before the
    //// centre of the sphere.
    //float3 v = pos - retval.CameraPosition.xyz;
    //float lv = length(v);
    //float dv = (rad * rad) / lv;
    //retval.Position.xyz -= dv * (v / lv);

    // Orient the sprite towards the camera.
    float4x4 matOrient = OrientToCamera(pos, invVm);
    retval.Position = mul(matOrient, retval.Position);

    // Move sprite to world position.
    retval.Position.xyz += pos;
    retval.Position.xyz -= matOrient._13_23_33 * rad;

#if defined(PER_VERTEX_RAY)
    retval.Ray = retval.Position.xyz - retval.CameraPosition.xyz;
#endif /* defined(PER_VERTEX_RAY) */

    // Do the camera transform.
    retval.Position = mul(mvp, retval.Position);

    // Transform camera to glyph space.
    retval.CameraPosition.xyz -= pos;

    // Pass on world-space parameters for raycasting.
    retval.SphereParams = float4(pos, rad);

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
