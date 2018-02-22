/// <copyright file="SphereHullShaderCore.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "SpherePipeline.hlsli"


// The number of control points, which is one per (hemi-) sphere.
#define CNT_CONTROL_POINTS (1)

// The partitioning scheme used in the hull shader.
//#define PARTITIONING_SCHEME "fractional_odd"
#define PARTITIONING_SCHEME "integer"


#define VsOutput VsPassThroughOutput


/// <summary>
/// Compute the tessellation factors for the sphere or sprite.
/// </summary>
HsConstants CalcConstants(InputPatch<VsOutput, CNT_CONTROL_POINTS> patch,
        uint patchId : SV_PrimitiveID) {
    HsConstants retval = (HsConstants) 0;

#if defined(HOLOMOL)
    const uint eye = patch[0].Eye;
#else /* defined(HOLOMOL) */
    const uint eye = 0;
#endif /* defined(HOLOMOL) */

#if defined(QUAD_TESS)
    // Tessellation factor for a quad is fixed.
    retval.EdgeTessFactor[0]
        = retval.EdgeTessFactor[1]
        = retval.EdgeTessFactor[2]
        = retval.EdgeTessFactor[3]
        = retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = 4.0f;
    retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = 1.0f;

#elif (defined(POLY_TESS) || defined(SPHERE_TESS) || defined(HEMISPHERE_TESS))
    // Tessellation factors are CPU-defined.
    retval.EdgeTessFactor[0] = EdgeTessFactor.x;
    retval.EdgeTessFactor[1] = EdgeTessFactor.y;
    retval.EdgeTessFactor[2] = EdgeTessFactor.z;
    retval.EdgeTessFactor[3] = EdgeTessFactor.w;
    retval.InsideTessFactor[0] = InsideTessFactor.x;
    retval.InsideTessFactor[1] = InsideTessFactor.y;

#elif defined(ADAPT_POLY_TESS)
    /* Dynamic polygon. */
    float4x4 pm = ProjMatrix[eye];
    float4x4 vm = ViewMatrix[eye];
    float4x4 mvp = ViewProjMatrix[eye];

    float4 pos = float4(patch[0].SphereParams.xyz, 1.0f);
    pos = mul(pos, mvp);

    float4 rad = float4(patch[0].SphereParams.w, 0.f, 0.f, 1.f);
    rad = mul(rad, vm);

    float tessFactor = clamp(
        length(rad) / pos.w * pm._11 * AdaptiveTessParams.z,
        AdaptiveTessParams.x, AdaptiveTessParams.y);

    retval.EdgeTessFactor[0]
        = retval.EdgeTessFactor[1]
        = retval.EdgeTessFactor[2]
        = retval.EdgeTessFactor[3]
        = retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = tessFactor;
    retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = 1.0f;

#elif (defined(ADAPT_SPHERE_TESS) || defined(ADAPT_HEMISPHERE_TESS))
    /* Dynamic geometry tessellation used by HoliMoli: */
    float4x4 pm = ProjMatrix[eye];
    float4x4 vm = ViewMatrix[eye];
    float4x4 mvp = ViewProjMatrix[eye];

    float4 pos = float4(patch[0].SphereParams.xyz, 1.0f);
    pos = mul(pos, mvp);

    float4 rad = float4(patch[0].SphereParams.w, 0.f, 0.f, 1.f);
    rad = mul(rad, vm);

    // using w-value that is equal to the z-component prior to the projection
    float tessFactor = clamp(
        length(rad) / pos.w * pm._11 * AdaptiveTessParams.z,
        AdaptiveTessParams.x, AdaptiveTessParams.y);
#if defined(ADAPT_HEMISPHERE_TESS)
    tessFactor *= HemisphereTessScaling;
#endif /* defined(ADAPT_HEMISPHERE_TESS) */

    retval.EdgeTessFactor[0]
        = retval.EdgeTessFactor[1]
        = retval.EdgeTessFactor[2]
        = retval.EdgeTessFactor[3]
        = retval.InsideTessFactor[0]
        = retval.InsideTessFactor[1] = tessFactor;
#endif

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
