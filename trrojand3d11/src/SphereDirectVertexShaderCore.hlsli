/// <copyright file="SphereDirectVertexShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "ColourConversion.hlsli"
#include "LocalLighting.hlsli"
#include "OrientToCamera.hlsli"
#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"
#include "TransferFunction.hlsli"


#ifdef PER_VERTEX_TRANSFER_FUNCTION
/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);

/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);
#endif /* PER_VERTEX_TRANSFER_FUNCTION */


#define VsInput VsRaycastingInput


/// <summary>
/// Core vertex shader implementation for particle rendering from the vertex
/// buffer.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;
    // TODO: PER_VERTEX_RAY

#if defined(PER_VERTEX_RADIUS)
    /* Sphere parameters are completely stored in vertex buffer. */
    retval.SphereParams = input.Position;
#else /* defined(PER_VERTEX_RADIUS) */
    /* Only position is in vertex buffer, add global radius from constants. */
    retval.SphereParams = float4(input.Position.xyz, GlobalRadius);
#endif /* defined(PER_VERTEX_RADIUS) */

#if (defined(PER_VERTEX_INTENSITY) && defined(PER_VERTEX_TRANSFER_FUNCTION))
    /* Per-vertex intensity is transformed to colour in vertex shader.*/
    float intensity = input.Intensity;
    float texCoords = TexCoordsFromIntensity(intensity, IntensityRange);
    retval.Colour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
#elif defined(PER_VERTEX_INTENSITY)
    /* Per-vertex intensity is passed trough to pixel shader. */
    float intensity = input.Intensity;
    retval.Colour = intensity.rrrr;
#elif defined(PER_VERTEX_COLOUR)
    /* Per-vertex byte colour is passed through to pixel shader. */
    retval.Colour = input.Colour;
#else
    /* Global colour is used for all vertices.*/
    retval.Colour = GlobalColour;
#endif

#if defined(HOLOMOL)
    retval.Eye = input.Eye;
#endif /* defined(HOLOMOL) */

    return retval;
}
