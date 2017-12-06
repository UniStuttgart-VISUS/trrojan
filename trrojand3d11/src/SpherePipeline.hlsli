/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

struct VsInput {
    float4 Position : POSITION;
#ifdef PER_VERTEX_COLOUR
    float4 Colour : COLOR;
#endif /* PER_VERTEX_COLOUR */
};

struct VsOutput {
    float4 Position : POSITION;
    float4 Colour : COLOR;
    float Radius : FOG;
};

struct HsConstants {
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

struct GsInput {
    float4 Position : POSITION;
    float4 Colour : COLOR;
    float4 SphereParams : TEXCOORD0;
};

struct PsInput {
    float4 Position : SV_POSITION;
    float4 SphereParams : TEXCOORD0;
    float4 Colour : COLOR0;
    nointerpolation float4 CameraPosition : TEXCOORD1;
    nointerpolation float4 CameraDirection : TEXCOORD2;
    nointerpolation float4 CameraUp : TEXCOORD3;
    nointerpolation float4 CameraRight : TEXCOORD4;
    //nointerpolation float EyeSeparation : TESSFACTOR0;
    //nointerpolation float Convergence : TESSFACTOR1;
};

struct PsOutput {
    float4 Colour : SV_TARGET;
    float Depth : SV_DEPTH;
};

#endif /* _MSC_VER */

cbuffer SphereConstants CBUFFER(0) {
    float4x4 ViewMatrix;
    float4x4 ProjMatrix;
    float4x4 ViewProjMatrix;
    float4x4 ViewInvMatrix;
    float4x4 ViewProjInvMatrix;
    float4 Viewport;
    float4 GlobalColour;
    float4 IntensityRangeAndGlobalRadius;
};

