// <copyright file="VolumeCamera.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
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

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */
