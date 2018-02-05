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

#ifdef PER_VERTEX_RADIUS
    /* Sphere parameters are completely stored in vertex buffer. */
    retval.SphereParams = input.Position;
#else /* PER_VERTEX_RADIUS */
    /* Only position is in vertex buffer, add global radius from constants. */
    retval.SphereParams = float4(input.Position.xyz, GlobalRadius);
#endif /* PER_VERTEX_RADIUS */

#ifdef PER_VERTEX_COLOUR
    /* Per-vertex colour is given in vertex buffer. */
    retval.Colour = input.Colour;

#elif PER_VERTEX_INTENSITY
#ifdef PER_VERTEX_TRANSFER_FUNCTION
    /* Per-vertex intensity is transformed to colour in vertex shader.*/
    float texCoords = TexCoordsFromIntensity(input.Intensity.r, IntensityRange);
    retval.Colour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);
#else /* PER_VERTEX_TRANSFER_FUNCTION */
    /* Per-vertex intensity is passed trough to pixel shader. */
    retval.Colour = input.Intensity.rrrr;
#endif /* PER_VERTEX_TRANSFER_FUNCTION */

#else /* PER_VERTEX_COLOUR */
    /* No vertex colour is given, use global one from constant buffer. */
    retval.Colour = GlobalColour;
#endif /* PER_VERTEX_COLOUR */

#ifdef HOLOMOL
    /* HoloLens needs to pass through the eye for later matrix accesses. */
    retval.Eye = input.Eye;
#endif /* HOLOMOL */

    return retval;
}
