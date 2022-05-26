// <copyright file="OverlayPixelShader.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// The texture to be drawn on top of the surface.
/// </summary>
Texture2D Texture: register(t0);

/// <summary>
/// Linear sampler for lookup up <see cref="Texture" />.
/// </summary>
SamplerState LinearSampler : register(s0);


/// <summary>
/// Pixel shader for compositing a texture on top of a surface.
/// </summary>
float4 Main(float4 position : SV_POSITION, float2 texCoords : TEXCOORD0) : SV_TARGET {
    float4 retval = Texture.SampleLevel(LinearSampler, texCoords, 0);
    return retval;
}
