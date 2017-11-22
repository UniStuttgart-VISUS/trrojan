/// <copyright file="DebugPixelShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "DebugPipeline.hlsli"

Texture2D ImgTexture;

SamplerState LinearSampler : register(s0);

/// <summary>
/// Entry point of the pixel shader.
/// </summary>
float4 Main(PsInput input) : SV_TARGET{
    float4 retval = float4(0.0f, 0.0f, 0.0f, 0.0f);
    retval = ImgTexture.Sample(LinearSampler, input.TexCoords);
    return retval;
}
