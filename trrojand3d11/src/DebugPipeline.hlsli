/// <copyright file="DebugPipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

/// <summary>
/// Input to vertex shader stage.
/// </summary>
struct VsInput {
    uint Id : SV_VERTEXID;
};

/// <summary>
/// Output of vertex shader stage and input to pixel shader.
/// </summary>
struct PsInput {
    float4 Position : SV_POSITION;
    float2 TexCoords : TEXCOORD0;
};
#endif /* _MSC_VER */

cbuffer DebugConstants CBUFFER(0) {
    float4 Viewport;
};
