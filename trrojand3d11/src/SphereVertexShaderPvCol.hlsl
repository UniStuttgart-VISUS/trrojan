/// <copyright file="SphereVertexShaderPvCol.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#define PER_VERTEX_COLOUR 1
#include "SpherePipeline.hlsli"

#define VsInput VsRaycastingInput


/// <summary>
/// Vertex shader for constant radius, but per-vertex colour.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.SphereParams = float4(input.Position.xyz, GlobalRadius);
    retval.Colour = input.Colour;
#ifdef HOLOMOL
    retval.Eye = input.Eye;
#endif /* HOLOMOL */

    return retval;
}
