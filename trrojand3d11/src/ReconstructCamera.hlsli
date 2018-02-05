/// <copyright file="ReconstructCamera.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


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
    // calculate cam position
    pos = viewInvMatrix._41_42_43_44; // (C) by Christoph

    dir = float4(normalize(viewInvMatrix._31_32_33), 0.0);
    up = normalize(viewInvMatrix._21_22_23_24);
    right = float4(normalize(cross(dir.xyz, up.xyz)), 0.0);

    up = float4(normalize(cross(right.xyz, dir.xyz)), 0.0);
}
