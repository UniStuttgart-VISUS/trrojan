/// <copyright file="SinglePassVolumePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

/// <summary>
/// The output of the pixel shader stage.
/// </summary>
struct PsOutput {
    float4 Colour : SV_TARGET;
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
    float4 CameraPosition;
    float4 CameraDirection;
    float4 CameraUp;
    float4 CameraRight;
    float2 ClippingPlanes;
    float2 FieldOfView;
    uint2 ImageSize;
};


cbuffer RaycastingConstants CBUFFER(1) {
    float4 BoxMin;
    float4 BoxMax;
    float ErtThreshold;
    float StepSize;
    float MaxValue;
    uint StepLimit;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */
