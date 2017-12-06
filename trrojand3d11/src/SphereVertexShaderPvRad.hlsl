/// <copyright file="SphereVertexShaderPvRad.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


/// <summary>
/// Pass-through pixel shader which only separates the radius from the position
/// for further processing in the geometry shader using a global, constant
/// colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.Position = float4(input.Position.xyz, 1.0f);
    retval.Radius = input.Position.w;
    retval.Colour = GlobalColour;
#ifdef HOLOMOL
    retval.Eye = input.Eye;
#endif /* HOLOMOL */

    return retval;
}
