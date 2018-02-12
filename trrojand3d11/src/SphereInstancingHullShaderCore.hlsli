/// <copyright file="SphereInstancingHullShaderCore.hlsl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#include "SpherePipeline.hlsli"


// The number of control points, which is one per splat.
#define CNT_CONTROL_POINTS (1)


/// <summary>
/// Compute the tessellation factors for the sprite to raycast the sphere on.
/// </summary>
HsConstants CalcConstants(InputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
    uint patchId : SV_PrimitiveID) {
    HsConstants retval = (HsConstants) 0;

#ifdef HOLOMOL
    const uint eye = patch[0].Eye;
#else /* HOLOMOL */
    const uint eye = 0;
#endif /* HOLOMOL */

    float4x4 pm = ProjMatrix[eye];
    float4x4 mvp = ViewProjMatrix[eye];
    float rad = patch[0].Radius;

    float4 pos = patch[0].Position;
    pos = mul(pos, mvp);

    float4 r = float4(rad, 0.f, 0.f, 1.f);
    r = mul(r, mvp);
    //r = mul(r, viewProjection[ip[0].instId]);
    //r.xyz = r / r.w;

    // using w-value that is equal to the z-component prior to the projection
    float tessFactor = clamp(length(r) / pos.w * pm._11 * 3.f, 5.f, 25.f);
    //float tessFactor = clamp(length(r) / pos.w * projection._22 * 2.f, 5.f, 25.f);

    // TODO: adding dynamic tessellation factors
    retval.EdgeTessFactor[0]
        = retval.EdgeTessFactor[1]
        = retval.EdgeTessFactor[2]
        = retval.EdgeTessFactor[3]
        = retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = IntRangeGlobalRadTessFactor.w;
    retval.InsideTessFactor[0] = 1;
    retval.InsideTessFactor[1] = 1;

    return retval;
}


/// <summary>
/// Hull shader for creating a splat for later raycasting.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
[domain("quad")]
[patchconstantfunc("CalcConstants")]
[partitioning("fractional_odd")]
[outputcontrolpoints(CNT_CONTROL_POINTS)]
[outputtopology("triangle_ccw")]
VsOutput Main(InputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        uint pointId : SV_OutputControlPointID,
        uint patchId : SV_PrimitiveID) {
    VsOutput retval = (VsOutput) 0;
    retval = patch[pointId];
    return retval;
}
