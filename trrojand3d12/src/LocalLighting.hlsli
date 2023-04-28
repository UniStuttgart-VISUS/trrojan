// <copyright file="LocalLighting.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// Performs pixel shading as in MegaMol.
/// </summary>
/// <param name="ray">The eye to fragment ray vector.</param>
/// <param name="normal">The normal of this fragment.</param>
/// <param name="lightPos">The position of the light source.</param>
/// <param name="colour">The base material colour.</param>
/// <returns>The lit colour of the pixel</returns>
float3 LocalLighting(const in float3 ray, const in float3 normal,
    const in float3 lightPos, const in float3 colour) {
    // TODO: rewrite!

    float3 lightDir = normalize(lightPos);

    float4 lightparams = float4(0.2, 0.8, 0.4, 10.0);
#define LIGHT_AMBIENT lightparams.x
#define LIGHT_DIFFUSE lightparams.y
#define LIGHT_SPECULAR lightparams.z
#define LIGHT_EXPONENT lightparams.w

    float nDOTl = dot(normal, lightDir);

    float3 r = normalize(2.0 * nDOTl * normal - lightDir);
    return LIGHT_AMBIENT * colour
        + LIGHT_DIFFUSE * colour * max(nDOTl, 0.0)
        + LIGHT_SPECULAR * float3(pow(max(dot(r, -ray), 0.0), LIGHT_EXPONENT).xxx);
}
