/// <copyright file="RayPassPixelShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "TwoPassVolumePipeline.hlsli"


/// <summary>
/// Pixel shader for the ray computation pass of two-pass volume rendering.
/// </summary>
/// <remarks>
/// The shader assumes an additive blending state being active and scales the
/// incoming fragments according to whether they are a front face or a back
/// face of the unit-sized cube.
/// </remarks>
/// <param name="input"></param>
/// <returns></returns>
PsOutput Main(PsInput input, bool isFrontFace : SV_IsFrontFace) {
    PsOutput retval = (PsOutput) 0;

    float scale = isFrontFace ? -1.0f : 1.0f;
    //scale = isFrontFace ? 1.0f : 0.0f;
    //scale = isFrontFace ? 0.0f : 1.0f;
    retval.Ray = scale * input.Ray;

    return retval;
}
