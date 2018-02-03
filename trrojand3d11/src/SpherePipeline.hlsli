/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

// CONSERVATIVE_DEPTH
// PER_VERTEX_COLOUR

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */

struct VsInput {
    float4 Position : POSITION;
#ifdef PER_VERTEX_COLOUR
    float4 Colour : COLOR;
#endif /* PER_VERTEX_COLOUR */
#ifdef HOLOMOL
    uint Eye : SV_InstanceID;
#endif /* HOLOMOL */
};

struct VsOutput {
    float4 Position : POSITION;
    float4 Colour : COLOR;
    float Radius : FOG;
#ifdef HOLOMOL
    uint Eye: TEXCOORD0;
#endif /* HOLOMOL */
};

struct HsConstants {
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};


/// <summary>
/// The input of the pixel shader stage.
/// </summary>
struct PsInput {
#ifdef CONSERVATIVE_DEPTH
    linear noperspective sample float4 Position : SV_POSITION;
#else /* CONSERVATIVE_DEPTH */
    float4 Position : SV_POSITION;
#endif /* CONSERVATIVE_DEPTH */
    float4 SphereParams : TEXCOORD0;
    float4 Colour : COLOR0;
    nointerpolation float4 CameraPosition : TEXCOORD1;
    nointerpolation float4 CameraDirection : TEXCOORD2;
    nointerpolation float4 CameraUp : TEXCOORD3;
    nointerpolation float4 CameraRight : TEXCOORD4;
#ifdef HOLOMOL
    nointerpolation uint Eye: SV_RenderTargetArrayIndex;
#endif /* HOLOMOL */
};


/// <summary>
/// The output of the pixel shader stage.
/// </summary>
struct PsOutput {
    float4 Colour : SV_TARGET;
#ifdef CONSERVATIVE_DEPTH
    float Depth : SV_DepthGreaterEqual;
#else /* CONSERVATIVE_DEPTH */
    float Depth : SV_DEPTH;
#endif /* CONSERVATIVE_DEPTH */
};
#endif /* _MSC_VER */


#ifdef HOLOMOL
#define STEREO_BUFFERS (2)
#else /* HOLOMOL */
#define STEREO_BUFFERS (1)
#endif /* HOLOMOL */


cbuffer SphereConstants CBUFFER(0) {
    float4x4 ViewMatrix[STEREO_BUFFERS];
    float4x4 ProjMatrix[STEREO_BUFFERS];
    float4x4 ViewProjMatrix[STEREO_BUFFERS];
    float4x4 ViewInvMatrix[STEREO_BUFFERS];
    float4x4 ViewProjInvMatrix[STEREO_BUFFERS];
    float4 Viewport;
    float4 GlobalColour;
    float4 IntRangeGlobalRadTessFactor;
};

