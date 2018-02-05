/// <copyright file="TransferFunction.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>


/// <summary>
/// Computes 1D texture coordinates for an intensity value.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
float TexCoordsFromIntensity(float intensity, float2 range) {
    float s = min(range.x, range.y);
    float r = abs(range.y - range.x);
    float retval = (intensity - s) / r;
    return retval;
}
