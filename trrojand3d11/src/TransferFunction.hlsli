/// <copyright file="TransferFunction.hlsli" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
