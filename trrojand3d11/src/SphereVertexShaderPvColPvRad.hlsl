/// <copyright file="SphereVertexShaderPvColPvRad.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#define PER_VERTEX_COLOUR 1
#include "SpherePipeline.hlsli"


/// <summary>
/// Vertex shader for per-vertex radius and colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = input.Position.w;
    retval.Colour = input.Colour;

    return retval;
}
