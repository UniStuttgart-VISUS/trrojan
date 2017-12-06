/// <copyright file="SphereVertexShaderPvInt.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#define PER_VERTEX_COLOUR 1
#include "SpherePipeline.hlsli"


/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t0);


/// <summary>
/// Linear sampler for transfer function lookup.
/// </summary>
SamplerState LinearSampler : register(s0);


/// <summary>
/// Vertex shader for constant radius and mapped colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = IntRangeGlobalRadTessFactor.z;

    float start = min(IntRangeGlobalRadTessFactor.x, IntRangeGlobalRadTessFactor.y);
    float range = abs(IntRangeGlobalRadTessFactor.y - IntRangeGlobalRadTessFactor.x);
    float texCoords = (input.Colour.r - start) / range;

    retval.Colour = TransferFunction.SampleLevel(LinearSampler, texCoords, 0);

    return retval;
}
