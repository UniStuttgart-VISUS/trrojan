/// <copyright file="RayPassVertexShader.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "TwoPassVolumePipeline.hlsli"


/// <summary>
/// Vertex shader for the ray computation pass in two-pass volume rendering.
/// </summary>
/// <remarks>
/// The shader assumes the geometry being a unit-sized cube around the origin of
/// the coordinate system.
/// </remarks>
/// <param name="input"></param>
/// <returns></returns>
VsOutput Main(VsInput input) {
    VsOutput retval = (VsOutput) 0;

    retval.TexCoords = float4(input.Position, 1.0f);
    retval.Position = mul(retval.TexCoords, ViewProjMatrix);
    retval.TexCoords.xyz += 0.5f.xxx;
    retval.TexCoords.w = 0.0f;

    return retval;
}
