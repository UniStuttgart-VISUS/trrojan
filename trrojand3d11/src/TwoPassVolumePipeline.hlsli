/// <copyright file="TwoPassVolumePipeline.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

/// <summary>
/// The input of the ray vertex shader.
/// </summary>
struct VsInput {
    float3 Position : POSITION0;
};

/// <summary>
/// The output of the ray vertex shader.
/// </summary>
struct VsOutput {
    float4 Position : SV_Position;
    float4 TexCoords: TEXCOORD0;
};

/// <summary>
/// The input of the ray pixel shader.
/// </summary>
typedef VsOutput PsInput;

/// <summary>
/// The output of the pixel shader stage.
/// </summary>
struct PsOutput {
    float4 EntryPoint : SV_TARGET0;
    float4 Ray : SV_TARGET1;
};

#endif /* _MSC_VER */


#ifdef HOLOMOL
#define STEREO_BUFFERS (2)
#else /* HOLOMOL */
#define STEREO_BUFFERS (1)
#endif /* HOLOMOL */


#ifdef _MSC_VER
// See https://msdn.microsoft.com/de-de/library/windows/desktop/bb509632(v=vs.85).aspx
#pragma pack(push)
#pragma pack(4)
#endif /* _MSC_VER */


cbuffer ViewConstants CBUFFER(0) {
    //float4x4 ViewMatrix;
    //float4x4 ProjectionMatrix;
    float4x4 ViewProjMatrix;
};


cbuffer RaycastingConstants CBUFFER(1) {
    float ErtThreshold;
    float StepSize;
    uint2 ImageSize;
    uint StepLimit;
    uint3 _Padding;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */
