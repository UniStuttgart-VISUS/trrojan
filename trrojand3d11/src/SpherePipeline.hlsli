/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

#ifdef _MSC_VER
#pragma once
#else /* _MSC_VER */


/// <summary>
/// A geometry vertex shader input for instancing.
/// </summary>
struct VsGeometryInput {
    /// <summary>
    /// The instance-relative position of the vertex.
    /// </summary>
    float3 Position : SV_Position;

    /// <summary>
    /// The vertex normal.
    /// </summary>
    float3 Normal : NORMAL;

    /// <summary>
    /// The instance ID which identifies the particle in the structured
    /// resource view.
    /// </summary>
    uint InstanceID : SV_InstanceID;
};


/// <summary>
/// Empty vertex input for instancing something from nothing.
/// </summary>
struct VsNoInput {
    /// <summary>
    /// The instance ID which identifies the particle in the structured
    /// resource view.
    /// </summary>
    uint InstanceID : SV_InstanceID;

    /// <summary>
    /// The instance-relative vertex ID which determines the sprite-relative
    /// location of the vertex to be generated.
    /// </summary>
    uint VertexID : SV_VertexID;
};


/// <summary>
/// An element from the vertex buffer, which is in most of the cases a
/// particle except for instancing-based methods reading the actual particle
/// data from a structured buffer view.
/// </summary>
struct VsRaycastingInput {
    /// <summary>
    /// The world-space position (xyz) and radius (w) of the particle.
    /// </summary>
    float4 SphereParams : POSITION;

#if defined(PER_VERTEX_COLOUR)
    /// <summary>
    /// The colour of the particle.
    /// </summary>
    float4 Colour : COLOR0;

#elif (defined(PER_VERTEX_INTENSITY) || defined(PER_PIXEL_INTENSITY))
    /// <summary>
    /// A scalar intensity which is transformed into a colour by means of
    /// a transfer function, either in the vertex or in the pixel shader.
    /// </summary>
    float Intensity : COLOR0;
#endif /* defined(PER_VERTEX_COLOUR) */

#if defined(HOLOMOL)
    /// <summary>
    /// The instance ID, which is required to determine the correct render
    /// target on HoloLens.
    /// </summary>
    uint Eye : SV_InstanceID;
#endif /*  defined(HOLOMOL) */
};


/// <summary>
/// A particle from the structured buffer.
/// </summary>
struct Particle {
#if defined(PER_VERTEX_RADIUS)
    /// <summary>
    /// The world-space position (xyz) and radius (w) of the particle.
    /// </summary>
    float4 SphereParams;
#else /* defined(PER_VERTEX_RADIUS) */
    float3 SphereParams;
#endif /* defined(PER_VERTEX_RADIUS) */

#if defined(PER_VERTEX_COLOUR)
    /// <summary>
    /// The colour of the particle.
    /// </summary>
#ifdef FLOAT_COLOUR
    float4 Colour;
#else /* FLOAT_COLOUR */
    uint Colour;
#endif /* FLOAT_COLOUR */

#elif (defined(PER_PIXEL_INTENSITY) || defined(PER_VERTEX_INTENSITY))
    /// <summary>
    /// A scalar intensity which is transformed into a colour by means of
    /// a transfer function.
    /// </summary>
    float Intensity;
#endif /* defined(PER_VERTEX_COLOUR) */
};


/// <summary>
/// Pass-through output of a vertex shader followed by a geometry or
/// tessellation shader stage that computes the actual vertex positions.
/// </summary>
struct VsPassThroughOutput {
    /// <summary>
    /// The world-space position (xyz) and radius (w) of the particle.
    /// </summary>
    float4 SphereParams : SV_POSITION;

#if defined(PER_PIXEL_INTENSITY)
    /// <summary>
    /// The intensity value to be processed using a transfer function.
    /// </summary>
    float Intensity : COLOR0;
#else /*defined(PER_PIXEL_INTENSITY) */
    /// <summary>
    /// The (base) colour of the pixel before shading.
    /// </summary>
    float4 Colour : COLOR0;
#endif /*defined(PER_PIXEL_INTENSITY) */

#if defined(HOLOMOL)
    uint Eye: TEXCOORD0;
#endif /* defined(HOLOMOL) */
};


/// <summary>
/// The constant output of the hull shader.
/// </summary>
struct HsConstants {
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};


/// <summary>
/// The output of the pixel shader in case of rendering actual sphere geometry.
/// </summary>
struct PsGeometryInput {
    /// <summary>
    /// Position of the pixel.
    /// </summary>
    float4 Position : SV_POSITION;

#if defined(PER_PIXEL_INTENSITY)
    /// <summary>
    /// The intensity value to be processed using a transfer function.
    /// </summary>
    float Intensity : COLOR0;
#else /*defined(PER_PIXEL_INTENSITY) */
    /// <summary>
    /// The (base) colour of the pixel before shading.
    /// </summary>
    float4 Colour : COLOR0;
#endif /*defined(PER_PIXEL_INTENSITY) */

    /// <summary>
    /// The world-space normal used for shading.
    /// </summary>
    float3 WorldNormal : NORMAL0;

    /// <summary>
    /// The view vector in world-space.
    /// </summary>
    nointerpolation float4 ViewDirection : TEXCOORD0;

    ///// <summary>
    ///// The world-space position of the vertex.
    ///// </summary>
    //nointerpolation float3 WorldPosition : TEXCOORD1;

#if defined(HOLOMOL)
    nointerpolation uint Eye: SV_RenderTargetArrayIndex;
#endif /* defined(HOLOMOL) */
};


/// <summary>
/// The input of the pixel shader stage when performing raycasting of the
/// spheres.
/// </summary>
struct PsRaycastingInput {
    /// <summary>
    /// The position of the pixel.
    /// </summary>
#if defined(CONSERVATIVE_DEPTH)
    linear noperspective sample float4 Position : SV_POSITION;
#else /* defined(CONSERVATIVE_DEPTH) */
    float4 Position : SV_POSITION;
#endif /* defined(CONSERVATIVE_DEPTH) */

#if defined(PER_PIXEL_INTENSITY)
    /// <summary>
    /// The intensity value to be processed using a transfer function.
    /// </summary>
    float Intensity : COLOR0;
#else /*defined(PER_PIXEL_INTENSITY) */
    /// <summary>
    /// The (base) colour of the pixel before shading.
    /// </summary>
    float4 Colour : COLOR0;
#endif /*defined(PER_PIXEL_INTENSITY) */

    /// <summary>
    /// The world-space position (xyz) and radius (w) of the particle.
    /// </summary>
    float4 SphereParams : TEXCOORD0;

    nointerpolation float4 CameraPosition : TEXCOORD1;
    nointerpolation float4 CameraDirection : TEXCOORD2;
    nointerpolation float4 CameraUp : TEXCOORD3;
    nointerpolation float4 CameraRight : TEXCOORD4;

#if defined(PER_VERTEX_RAY)
    float3 Ray : TEXCOORD5;
#endif /* defined(PER_VERTEX_RAY) */

#if defined(HOLOMOL)
    nointerpolation uint Eye: SV_RenderTargetArrayIndex;
#endif /* defined(HOLOMOL) */
};


/// <summary>
/// The output of the pixel shader stage.
/// </summary>
struct PsOutput {
    float4 Colour : SV_TARGET;
#if defined(CONSERVATIVE_DEPTH)
    float Depth : SV_DepthGreaterEqual;
#elif defined(RAYCASTING)
    float Depth : SV_DEPTH;
#endif /* defined(CONSERVATIVE_DEPTH) */
};
#endif /* _MSC_VER */


#ifdef HOLOMOL
#define STEREO_BUFFERS (2)
#else /* HOLOMOL */
#define STEREO_BUFFERS (1)
#endif /* HOLOMOL */


#ifdef _MSC_VER
// See https://msdn.microsoft.com/de-de/library/windows/desktop/bb509632(v=vs.85).aspx
#pragma pack(push)
#pragma pack(4)
#endif /* _MSC_VER */

// Note: the current host implementation required the constant buffers to be the
// same for all techniques, because we are including all this header in exactly
// one file. Future implementations could work around this by using namespaces.

cbuffer SphereConstants CBUFFER(0) {
    float4 GlobalColour;
    float2 IntensityRange;
    float GlobalRadius;
    float _SpherePadding[1];

};

cbuffer ViewConstants CBUFFER(1) {
    float4x4 ViewMatrix[STEREO_BUFFERS];
    float4x4 ProjMatrix[STEREO_BUFFERS];
    float4x4 ViewProjMatrix[STEREO_BUFFERS];
    float4x4 ViewInvMatrix[STEREO_BUFFERS];
    float4x4 ViewProjInvMatrix[STEREO_BUFFERS];
    float4 Viewport;
};


cbuffer TessellationConstants CBUFFER(2) {
    /// <summary>
    /// Specifies the tessellation factors for the hull shader.
    /// </summary>
    /// <remarks>
    /// <para>In case of host-controlled tessellation, this variable specifies
    /// the edge tessellation factors. This is the case for non-adaptive sphere
    /// and hemisphere tessellation.</para>
    /// </remarks>
    float4 EdgeTessFactor;

    /// <summary>
    /// Specifies the tessellation factors for the hull shader.
    /// </summary>
    /// <remarks>
    /// <para>For non-adaptive sphere and hemisphere tessellation, this value
    /// specifies the inner tessellation factors of the quad wrapped around the
    /// sphere.</para>
    /// </remarks>
    float2 InsideTessFactor;

    float2 _TessPadding1;

    /// <summary>
    /// For tessellation of adaptive polygons, specifies the minimum (index 0)
    /// and maximum (index 1) number of corners as well as the scaling
    /// parameter of the adaptive tessellation (index 2).
    /// For adaptive tessellation of spheres, this first two indices specify
    /// the minimum and maximum tessellation factor whereas the third again
    /// controls the tessellation. The values used by the HoliMoli app are
    /// 5, 25 and 3.
    /// </summary>
    float AdaptiveTessMin;
    float AdaptiveTessMax;
    float AdaptiveTessScale;

    /// <summary>
    /// A scaling factor for computing the tessellation factor for hemispheres
    /// from the one for spheres. 0.5 is a reasonable value.
    /// </summary>
    float HemisphereTessScaling;

    /// <summary>
    /// In case of shader-generated polygons, the number of corners to create.
    /// </summary>
    uint PolygonCorners;

    float2 _TessPadding2;
};

#ifdef _MSC_VER
#pragma pack(pop)
#endif /* _MSC_VER */
