/// <copyright file="SinglePassVolumeComputeShader.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

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


/// <summary>
/// Entry point of the compute shader.
/// </summary>
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
        //Output[threadID.xy] = float4(dir, 1.0f);
        //Output[threadID.xy] = float4(BoxMax.xyz / 128.0f, 1.0f);
        Output[threadID.xy] = 0.0f.xxxx;
        return;
    }
    if (start < 0.0f) start = 0.0f; // Clamp to near plane.

    // March along ray from front to back, accumulating colour.
    const float3 step = StepSize * dir;

    float4 colour = 0.0f.xxxx;
    float3 pos = (camPos + start * dir - BoxMin.xyz) / volSize;
    float lambda = start;

    for (uint i = 0; (i < StepLimit) && (lambda < end); ++i) {
        float4 value = Volume.SampleLevel(LinearSampler, pos, 0);
        float4 emission = TransferFunction.SampleLevel(LinearSampler, value.x, 0);
        emission.rgb *= emission.a;
        colour += emission * (1.0f - colour.a);

        if (colour.a > ErtThreshold) {
            break;
        }

        lambda += StepSize * volSize;
        pos += step;
    }

    Output[threadID.xy] = colour;
    //Output[threadID.xy] = float4((pos - BoxMin.xyz) / volSize, 1.0f);
    //Output[threadID.xy] = float4(dir, 1.0f);
    //Output[threadID.xy] = float4((camPos + start * dir - BoxMin.xyz) / volSize, 1.0f);
}

#if 0
uint3 groupID : SV_GroupID
- Index of the group within the dispatch for each dimension

uint3 groupThreadID : SV_GroupThreadID
- Index of the thread within the group for each dimension

uint groupIndex : SV_GroupIndex
- A sequential index within the group that starts from 0 top left back and goes on to bottom right front

uint3 dispatchThreadID : SV_DispatchThreadID
- Global thread index within the whole dispatch
#endif

#if 0
/**
*	The CUDA Kernel for the rendering process
*/
__global__ void d_render(uint * d_output, uint imageW, uint imageH, float fovx, float fovy, float3 camPos, float3 camDir, float3 camUp, float3 camRight, float zNear,
    float density, float brightness, float transferOffset, float transferScale, float minVal, float maxVal,
    const float3 boxMin = make_float3(-1.0f, -1.0f, -1.0f), const float3 boxMax = make_float3(1.0f, 1.0f, 1.0f), cudaExtent volSize = make_cudaExtent(1, 1, 1)) {

    const int maxSteps = 450;

    const float tstep = (boxMax.x - boxMin.x) / static_cast<float>(maxSteps);
    const float opacityThreshold = 0.95f;

    // pixel coordinates
    uint x = blockIdx.x * blockDim.x + threadIdx.x;
    uint y = blockIdx.y * blockDim.y + threadIdx.y;

    if ((x >= imageW) || (y >= imageH)) return;

    // texture coordinates
    float u = (x / static_cast<float>(imageW)) * 2.0f - 1.0f;
    float v = (y / static_cast<float>(imageH)) * 2.0f - 1.0f;

    // calculate intersection with near plane in world space
    float3 oL = (tan(fovx * 0.5f) * zNear) * (-camRight) + (tan(fovy * 0.5) * zNear) * camUp + camDir * zNear + camPos;
    float3 uL = (tan(fovx * 0.5f) * zNear) * (-camRight) + (tan(fovy * 0.5) * zNear) * (-camUp) + camDir * zNear + camPos;
    float3 oR = (tan(fovx * 0.5f) * zNear) * camRight + (tan(fovy * 0.5) * zNear) * camUp + camDir * zNear + camPos;
    float3 uR = (tan(fovx * 0.5f) * zNear) * camRight + (tan(fovy * 0.5) * zNear) * (-camUp) + camDir * zNear + camPos;

    float3 targetL = lerp(uL, oL, (v + 1.0f) * 0.5f);
    float3 targetR = lerp(uR, oR, (v + 1.0f) * 0.5f);

    float3 target = lerp(targetL, targetR, (u + 1.0f) * 0.5f);

    // calculate eye ray in world space
    Ray eyeRay;
    eyeRay.o = camPos;
    eyeRay.d = normalize(target - camPos);

    // find intersection with box
    float tnear, tfar;
    int hit = intersectBox(eyeRay, boxMin, boxMax, &tnear, &tfar);
    if (!hit) {
        d_output[y*imageW + x] = rgbaFloatToInt(make_float4(0.0f));
        return;
    }

    if (tnear < 0.0f) tnear = 0.0f; // clamp to near plane

                                    // march along ray from front to back, accumulating colour
    float4 sum = make_float4(0.0f);
    float t = tnear;
    float3 pos = eyeRay.o + eyeRay.d * tnear;
    float3 step = eyeRay.d * tstep;
    float3 diff = boxMax - boxMin;

    for (int i = 0; i < maxSteps; i++) {
        // remap position to [0, 1] coordinates
        float3 samplePos;
        samplePos.x = (pos.x - boxMin.x) / diff.x;
        samplePos.y = (pos.y - boxMin.y) / diff.y;
        samplePos.z = (pos.z - boxMin.z) / diff.z;

        // read from 3D texture
        float sample = tex3D(tex, samplePos.x, samplePos.y, samplePos.z);

        // normalize the sample
        sample /= maxVal;

        float sampleCamDist = length(eyeRay.o - pos);

        // lookup in transfer function texture
        float4 col = tex1D(customTransferTex, (sample - transferOffset) * transferScale);

        // pre-multiply alpha
        col.x *= col.w;
        col.y *= col.w;
        col.z *= col.w;

        // "over" operator for front-to-back blending
        sum = sum + col * (1.0f - sum.w);

        // exit early if opaque
        if (sum.w > opacityThreshold) {
            break;
        }

        t += tstep;

        if (t > tfar) break;

        pos += step;
    }

    sum *= brightness;

    // write output color
    d_output[y * imageW + x] = rgbaFloatToInt(sum);
}
#endif
