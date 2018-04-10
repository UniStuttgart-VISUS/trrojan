/// <copyright file="VolumePassComputeShader.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "TwoPassVolumePipeline.hlsli"


/// <summary>
/// The texture with the pre-computed entry points.
/// </summary>
Texture2D EntryPoints : register(t2);

/// <summary>
/// Linear sampler for texture lookups.
/// </summary>
SamplerState LinearSampler : register(s0);

/// <summary>
/// The output texture.
/// </summary>
RWTexture2D<unorm float4> Output : register(u0);

/// <summary>
/// The texture with the pre-computed rays.
/// </summary>
Texture2D Rays : register(t3);

/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t1);

/// <summary>
/// The volume.
/// </summary>
Texture3D Volume : register(t0);


/// <summary>
/// Entry point of the volume raycasting compute shader.
/// </summary>
/// <remarks>
/// The compute shader requires two textures representing the ray entry points
/// and the directions of the rays to be filled by a pre-rendering pass.
/// </remarks>
[numthreads(16, 16, 1)]
void Main(uint3 threadID : SV_DispatchThreadID) {
    // Terminate threads which do not produce a pixel.
    if (any(threadID.xy >= ImageSize.xy)) {
        return;
    }

    const float3 ray = Rays[threadID.xy].xyz;
    const float len = length(ray);
    const float3 step = StepSize * normalize(ray);

    float4 colour = 0.0f.xxxx;
    float3 pos = EntryPoints[threadID.xy].xyz;

    for (uint i = 0; (i < StepLimit) && (i * StepSize < len); ++i) {
        float4 value = Volume.SampleLevel(LinearSampler, pos, 0);
        float4 emission = TransferFunction.SampleLevel(LinearSampler, value.x, 0);
        emission.rgb *= emission.a;
        colour += emission * (1.0f - colour.a);

        if (colour.a > ErtThreshold) {
            break;
        }

        pos += step;
    }

    Output[threadID.xy] = colour;
    Output[threadID.xy] = float4(Rays[threadID.xy].xyz, 1.0f);
}
