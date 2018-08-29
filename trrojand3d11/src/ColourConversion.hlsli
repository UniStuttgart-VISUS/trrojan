/// <copyright file="ColourConversion.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>


/// <summary>
/// Converts an 8-bit RGBA colour to <c>float4</c>.
/// </summary>
/// <param name=""></param>
/// <param name=""></param>
/// <returns></returns>
float4 UintToFloat4Colour(uint colour) {
    float4 retval;

    retval.r = (colour & 0xFF) / 255.0f;
    colour >>= 8;
    retval.g = (colour & 0xFF) / 255.0f;
    colour >>= 8;
    retval.b = (colour & 0xFF) / 255.0f;
    colour >>= 8;
    retval.a = colour / 255.0f;

    return retval;
}
