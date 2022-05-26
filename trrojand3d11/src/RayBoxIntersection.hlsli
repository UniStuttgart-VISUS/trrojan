// <copyright file="RayBoxIntersection.hlsli" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/// <summary>
/// Performs the infamous ray-box intersection algorithm by Kay and Kayjia
/// described at https://www.siggraph.org//education/materials/HyperGraph/raytrace/rtinter3.htm
/// </summary>
/// <param name="rayOrigin">The start position of the ray.</param>
/// <param name="rayDirection">The direction vector of the ray.</param>
/// <param name="boxMin">The minimum values of the box on all three coordinate
/// axes.</param>
/// <param name="boxMax">The maximum values of the box on all three coordinate
/// axes.</param>
/// <param name="outNear">Receives the distance to the nearest intersection
/// point.</param>
/// <param name="outFar">Receives the distance to the farthest intersection
/// point.</param>
/// <returns><c>true</c> if the ray and the box intersect,
/// <c>false</c> otherwise.</returns>
bool RayBoxIntersection(float3 rayOrigin, float3 rayDirection, float3 boxMin,
        float3 boxMax, out float outNear, out float outFar) {
    // compute intersection of ray with all six bbox planes
    float3 invR = 1.0f.xxx / rayDirection;
    float3 tbot = invR * (boxMin - rayOrigin);
    float3 ttop = invR * (boxMax - rayOrigin);

    // re-order intersections to find smallest and largest on each axis
    float3 tmin = min(ttop, tbot);
    float3 tmax = max(ttop, tbot);

    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

    outNear = largest_tmin;
    outFar = smallest_tmax;

    return (smallest_tmax > largest_tmin);
}
