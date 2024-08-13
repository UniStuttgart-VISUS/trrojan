// <copyright file="SinglePassVolumeComputeShader.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "SinglePassVolumePipeline.hlsli"
#include "RayBoxIntersection.hlsli"


/// <summary>
/// Linear sampler for texture lookups.
/// </summary>
SamplerState LinearSampler : register(s0);

/// <summary>
/// The output texture.
/// </summary>
RWTexture2D<unorm float4> Output : register(u0);

/// <summary>
/// Transfer function.
/// </summary>
Texture1D TransferFunction : register(t1);

/// <summary>
/// The volume.
/// </summary>
Texture3D Volume : register(t0);


#define SHADER_ROOT_SIGNATURE \
"RootFlags(0),"\
"DescriptorTable(SRV(t0), SRV(t1), CBV(b0), CBV(b1), UAV(u0), visibility = SHADER_VISIBILITY_ALL),"\
"StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, filter = FILTER_MIN_MAG_MIP_LINEAR, visibility = SHADER_VISIBILITY_ALL)"


/// <summary>
/// Entry point of the compute shader.
/// </summary>
[RootSignature(SHADER_ROOT_SIGNATURE)]
[numthreads(16, 16, 1)]
void Main(uint3 threadID : SV_DispatchThreadID) {
    // Terminate threads which do not produce a pixel.
    if (any(threadID.xy >= ImageSize.xy)) {
        return;
    }

    const float3 camPos = CameraPosition.xyz;
    const float3 camDir = normalize(CameraDirection.xyz);
    const float3 camUp = normalize(CameraUp.xyz);
    const float3 camRight = normalize(CameraRight.xyz);
    const float2 fov = 0.5f * FieldOfView;
    const float3 volSize = abs(BoxMax.xyz - BoxMin.xyz);

    // Scale pixel coordinates to [0, 1].
    float2 uv = (float2) threadID / (float2) ImageSize;

    // Calculate intersection of ray with near plane in world space.
    float2 dNear = tan(fov) * ClippingPlanes.xx;
    float3 dx = dNear.x * camRight;
    float3 dy = dNear.y * camUp;
    float3 p = camDir * ClippingPlanes.x + camPos;
    float3 tl = p - dx + dy;
    float3 bl = p - dx - dy;
    float3 tr = p + dx + dy;
    float3 br = p + dx - dy;

    float3 il = lerp(bl, tl, uv.y);
    float3 ir = lerp(br, tr, uv.y);

    const float3 intersection = lerp(il, ir, uv.x);

    // Compute the ray in world space.
    const float3 dir = normalize(intersection - camPos);

    // Find intersection with data bounding box.
    float start, end;
    if (!RayBoxIntersection(camPos, dir, BoxMin.xyz, BoxMax.xyz, start, end)) {
        Output[threadID.xy] = 0.0f.xxxx;
        //Output[threadID.xy] = float4(dir, 1.0f);
        //Output[threadID.xy] = float4(BoxMax.xyz / 128.0f, 1.0f);
        return;
    }
    if (start < 0.0f) start = 0.0f; // Clamp to near plane.

    // March along ray from front to back, accumulating colour.
    const float3 step = StepSize * dir;

    float4 colour = 0.0f.xxxx;
    float3 pos = camPos + start * dir;
    float lambda = start;

    for (uint i = 0; (i < StepLimit) && (lambda < end); ++i) {
        float4 value = Volume.SampleLevel(LinearSampler, (pos - BoxMin.xyz) / volSize, 0);
        float4 emission = TransferFunction.SampleLevel(LinearSampler, value.x, 0);
        //emission = 0.1f.xxxx;
        emission.rgb *= emission.a;
        colour += emission * (1.0f - colour.a);

        if (colour.a > ErtThreshold) {
            break;
        }

        lambda += StepSize;
        pos += step;
    }

    Output[threadID.xy] = colour;
    //Output[threadID.xy] = float4((float3)threadID.xyy / ImageSize.xyy, 1.0f);
    //Output[threadID.xy] = TransferFunction.SampleLevel(LinearSampler, (float) threadID.x / ImageSize.x, 0);
    //Output[threadID.xy] = float4(Volume.SampleLevel(LinearSampler, float3((float2) threadID.xy / ImageSize.xy, 0.5f), 0).xyz, 1.0f);
    //Output[threadID.xy] = float4(abs(BoxMax.xyz) / 256.0f, 1.0f);
    //Output[threadID.xy] = float4(abs(CameraPosition.xyz), 1.0f);
    //Output[threadID.xy] = float4(abs(CameraRight.xyz), 1.0f);
    //Output[threadID.xy] = float2(ImageSize / 2048).xyxy;
    //Output[threadID.xy] = float2(1.0f, 0.0f).xyyx;
    //Output[threadID.xy] = float4((pos - BoxMin.xyz) / volSize, 1.0f);
    //Output[threadID.xy] = float4(start.xxx, 1.0f);
    //Output[threadID.xy] = float4(dir, 1.0f);
    //Output[threadID.xy] = float4((camPos + start * dir - BoxMin.xyz) / volSize, 1.0f);
}
