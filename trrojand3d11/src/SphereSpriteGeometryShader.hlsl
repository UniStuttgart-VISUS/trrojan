/// <copyright file="SphereSpriteGeometryShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


/// <summary>
/// Pass-through geometry shader for tessellated sprites.
/// </summary>
[maxvertexcount(3)]
void Main(triangle PsInput input[3], inout TriangleStream<PsInput> triStream) {

    [unroll(3)]
    for (int i = 0; i < 3; ++i) {
        triStream.Append(input[i]);
    }

    triStream.RestartStrip();
}
