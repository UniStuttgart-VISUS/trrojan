// <copyright file="ReconstructCamera.hlsl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// Reconstruct the camera coordinate axes from the inverse view matrix.
/// </summary>
/// <param name="pos"></param>
/// <param name="dir"></param>
/// <param name="up"></param>
/// <param name="right"></param>
/// <param name="viewInvMatrix"></param>
void ReconstructCamera(out float4 pos, out float4 dir, out float4 up,
        out float4 right, const in matrix viewInvMatrix) {
    // https://docs.microsoft.com/de-de/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math
    // Calculate cam position.
    pos = viewInvMatrix._14_24_34_44; // (C) by Christoph

    dir = float4(normalize(viewInvMatrix._13_23_33), 0.0);
    up = normalize(viewInvMatrix._12_22_32_42);
    right = float4(normalize(cross(dir.xyz, up.xyz)), 0.0);

    up = float4(normalize(cross(right.xyz, dir.xyz)), 0.0f);
}
