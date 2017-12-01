/// <copyright file="SphereVertexShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


/// <summary>
/// Vertex shader for constant radius and colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
GsInput Main(VsInput input) {
    GsInput retval = (GsInput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = IntensityRangeAndGlobalRadius.z;
    retval.Colour = GlobalColour;

    return retval;
}
