/// <copyright file="SphereHullShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


// The number of control points, which is one per (hemi-) sphere.
#define CNT_CONTROL_POINTS (1)

// The partitioning scheme used in the hull shader.
#define PARTITIONING_SCHEME "fractional_odd"


/// <summary>
/// Compute the tessellation factors for the sphere.
/// </summary>
HsConstants CalcConstants(InputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        uint patchId : SV_PrimitiveID) {
    HsConstants retval = (HsConstants) 0;

#ifdef HOLOMOL
    const uint eye = patch[0].Eye;
#else /* HOLOMOL */
    const uint eye = 0;
#endif /* HOLOMOL */

    /* Dynamic tessellation used by HoliMoli: */
    float4x4 pm = ProjMatrix[eye];
    float4x4 vm = ViewMatrix[eye];
    float4x4 mvp = ViewProjMatrix[eye];
    float rad = patch[0].Radius;

    float4 pos = patch[0].Position;
    pos = mul(pos, mvp);

    float4 r = float4(rad, 0.f, 0.f, 1.f);
    r = mul(r, vm);

    // using w-value that is equal to the z-component prior to the projection
    float tessFactor = clamp(length(r) / pos.w * pm._11 * 3.f, 5.f, 25.f);
#ifdef HEMISPHERE_TESSELLATION
    tessFactor /= 2.0f;
#endif HEMISPHERE_TESSELLATION

    // TODO: Host-controlled tess factors for sprites

    retval.EdgeTessFactor[0]
        = retval.EdgeTessFactor[1]
        = retval.EdgeTessFactor[2]
        = retval.EdgeTessFactor[3]
        = retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = tessFactor;

    return retval;
}


/// <summary>
/// Hull shader for passing the input to next stage.
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
[domain("quad")]
[patchconstantfunc("CalcConstants")]
[partitioning(PARTITIONING_SCHEME)]
[outputcontrolpoints(CNT_CONTROL_POINTS)]
[outputtopology("triangle_cw")]
VsOutput Main(InputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        uint pointId : SV_OutputControlPointID,
        uint patchId : SV_PrimitiveID) {
    VsOutput retval = (VsOutput) 0;
    retval = patch[pointId];
    return retval;
}
