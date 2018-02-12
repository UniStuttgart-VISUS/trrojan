/// <copyright file="SphereShadingRaycastingCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "LocalLighting.hlsli"
#include "SpherePipeline.hlsli"


#ifdef PER_PIXEL_TRANSFER_FUNCTION
/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);

/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);
#endif /* PER_PIXEL_TRANSFER_FUNCTION */


/// <summary>
/// Entry point of the pixel shader doing simple per-pixel shading for sphere
/// geometry.
/// </summary>
PsOutput Main(PsGeometryInput input) {
    PsOutput retval = (PsOutput) 0;

#ifdef PER_PIXEL_TRANSFER_FUNCTION
    // The colour is the intensity value, which for we need to perform a
    // texture lookup before shading.
    float texCoords = TexCoordsFromIntensity(input.Colour.r, IntensityRange);
    float4 baseColour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);

#else /* PER_PIXEL_TRANSFER_FUNCTION */
    // Colour is explicitly given or transfer function has already been
    // sampled in vertex shader stage.
    float4 baseColour = input.Colour;
#endif /* PER_PIXEL_TRANSFER_FUNCTION */

    float4 ray = normalize(input.ViewDirection);
    retval.Colour = float4(LocalLighting(-ray.xyz, input.Normal.xyz,
        ray.xyz, baseColour.rgb), baseColour.a);

    return retval;
}
