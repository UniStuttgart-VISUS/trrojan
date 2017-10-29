/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


struct VsInput {
    float4 Position : POSITION;
    float4 Colour : COLOR;
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


cbuffer Constants : register(b0) {
    matrix ProjMatrix;
    matrix ViewMatrix;
    matrix ViewInvMatrix;
    matrix ViewProjMatrix;
    matrix ViewProjInvMatrix;
    float4 Viewport;
    // TODO: Remove the following
    float4 CamPos;
    float4 CamDir;
    float4 CamUp;
};

