/// <copyright file="RayPassPixelShader.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "TwoPassVolumePipeline.hlsli"


/// <summary>
/// Pixel shader for the ray computation pass of two-pass volume rendering.
/// </summary>
/// <remarks>
/// The shader assumes an additive blending state being active and scales the
/// incoming fragments according to whether they are a front face or a back
/// face of the unit-sized cube around the origin. The coordinates must be
/// within [-0.5, 0.5] on all axes.
/// </remarks>
/// <param name="input"></param>
/// <returns></returns>
PsOutput Main(PsInput input, bool isFrontFace : SV_IsFrontFace) {
    PsOutput retval = (PsOutput) 0;

    retval.EntryPoint = isFrontFace ? input.TexCoords : 0.0f.xxxx;
    retval.Ray = isFrontFace ? -input.TexCoords : input.TexCoords;

    return retval;
}
