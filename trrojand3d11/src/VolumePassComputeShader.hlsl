/// <copyright file="VolumePassComputeShader.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "TwoPassVolumePipeline.hlsli"


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
Texture2D Rays : register(t2);

/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t1);

/// <summary>
/// The volume.
/// </summary>
Texture3D Volume : register(t0);


/// <summary>
/// Entry point of the compute shader.
/// </summary>
[numthreads(16, 16, 1)]
void Main(uint3 threadID : SV_DispatchThreadID) {
    // Terminate threads which do not produce a pixel.
    if (any(threadID.xy >= ImageSize.xy)) {
        return;
    }

    const float4 end = Rays[threadID.xy];
    const float4 dir = normalize(end);

    // Terminate all threads that did not hit the volume.
    if (length(end) == 0.0f) {
        Output[threadID.xy] = 0.0f.xxxx;
        return;
    }

#if 0
    // March along ray from front to back, accumulating colour.
    const float tstep = (BoxMax.x - BoxMin.x) / (float) StepLimit;
    float4 colour = 0.0f.xxxx;
    float3 physicalStep = tstep * dir;

    float lambda = tnear;
    float3 pos = camPos + lambda * dir;
    for (uint i = 0; (i < StepLimit) && (lambda < tfar); ++i) {
        float3 texCoords = (pos - BoxMin.xyz) / volSize;
        float4 intensity = Volume.SampleLevel(LinearSampler, texCoords, 0);
        //if (MaxValue != 0) intensity /= MaxValue;
        float4 emission = TransferFunction.SampleLevel(LinearSampler, intensity.x, 0);
        emission.rgb *= emission.a;
        colour += emission * (1.0f - colour.a);

        if (colour.a > ErtThreshold) {
            break;
        }

        lambda += tstep;
        pos += physicalStep;
    }
#endif

    Output[threadID.xy] = float4(end.xyz, 1.0f);
}
