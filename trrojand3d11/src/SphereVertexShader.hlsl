/// <copyright file="SphereVertexShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


/// <summary>
/// Pass-through pixel shader which only separates the radius from the position
/// for further processing in the geometry shader.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
GsInput Main(VsInput input) {
    GsInput retval = (GsInput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = input.Position.w;
    retval.Colour = input.Colour;
    retval.Colour = float4(1.0f, 0.0f, 0.0f, 1.0f);

    return retval;
}
