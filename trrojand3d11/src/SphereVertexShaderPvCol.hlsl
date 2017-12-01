/// <copyright file="SphereVertexShaderPvCol.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#define PER_VERTEX_COLOUR 1
#include "SpherePipeline.hlsli"


/// <summary>
/// Vertex shader for constant radius, but per-vertex colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
GsInput Main(VsInput input) {
    GsInput retval = (GsInput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = IntensityRangeAndGlobalRadius.z;
    retval.Colour = input.Colour;

    return retval;
}
