// <copyright file="SinglePassVolumePipeline.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
    uint StepLimit;
    uint _Padding;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */
