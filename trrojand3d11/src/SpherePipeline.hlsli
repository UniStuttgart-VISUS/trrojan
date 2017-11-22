/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

#define INPUT_POSITION_XYZR 1
#define INPUT_COLOUR 1

struct VsInput {
#ifdef INPUT_POSITION_XYZ
    float3 Position : POSITION;
#elif INPUT_POSITION_XYZR
    float4 Position : POSITION;
#else  /* INPUT_POSITION_XYZ */
#error No format was specified for the input position.
#endif /* INPUT_POSITION_XYZ */
#ifdef INPUT_COLOUR
    float4 Colour : COLOR;
#endif /* INPUT_COLOUR */
};

struct GsInput {
    float4 Position : POSITION;
    float4 Colour : COLOR;
    float Radius : FOG;
};

struct PsInput {
    float4 Position : SV_POSITION;
    float4 SphereParams : TEXCOORD0;
    float4 Colour : COLOR0;
    nointerpolation float4 CameraPosition : TEXCOORD1;
    nointerpolation float4 CameraDirection : TEXCOORD2;
    nointerpolation float4 CameraUp : TEXCOORD3;
    nointerpolation float4 CameraRight : TEXCOORD4;
    nointerpolation float EyeSeparation : TESSFACTOR0;
    nointerpolation float Convergence : TESSFACTOR1;
};

struct PsOutput {
    float4 Colour : SV_TARGET;
    float Depth : SV_DEPTH;
};

#endif /* _MSC_VER */

cbuffer SphereConstants CBUFFER(0) {
    float4x4 ViewProjMatrix;
    float4x4 ViewInvMatrix;
    float4x4 ViewProjInvMatrix;
    float4 Viewport;
};

