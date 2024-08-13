// <copyright file="OverlayVertexShader.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// Quad-from-nothing vertex shader.
/// </summary>
void Main(out float4 outPosition: SV_POSITION,
        out float2 outTexCoords : TEXCOORD0,
        in uint vertexID : SV_VertexID) {
    const float2 FLIP_Y = float2(1.0f, -1.0f);  // Flips the y-axis of a float2.
    outTexCoords = float2(vertexID % 2, vertexID / 2);
    outPosition = float4(outTexCoords.x, outTexCoords.y, 0.0f, 1.0f);
    outPosition.xy = 2.0f * FLIP_Y * (outPosition.xy - 0.5f.xx);
}
