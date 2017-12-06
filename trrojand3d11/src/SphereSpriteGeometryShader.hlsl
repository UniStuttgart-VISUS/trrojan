/// <copyright file="SphereSpriteGeometryShader.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "ReconstructCamera.hlsli"
#include "SpherePipeline.hlsli"


/// <summary>
/// 
/// </summary>
[maxvertexcount(3)]
void Main(triangle GsInput input[3], inout TriangleStream<PsInput> triStream) {
    PsInput v = (PsInput) 0;

    // Reconstruct camera system.
    ReconstructCamera(v.CameraPosition, v.CameraDirection, v.CameraUp,
        v.CameraRight, ViewInvMatrix);
    float3 camPos = v.CameraPosition.xyz;

    [unroll(3)]
    for (int i = 0; i < 3; ++i) {
        v.Position = input[i].Position;
        v.Colour = input[i].Colour;
        v.SphereParams = input[i].SphereParams;

        // Transform camera to glyph space.
        v.CameraPosition.xyz = camPos - input[i].SphereParams.xyz;

        triStream.Append(v);
    }

    triStream.RestartStrip();
}
