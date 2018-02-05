/// <copyright file="SpherePipeline.hlsli" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "hlsltypemapping.hlsli"

// CONSERVATIVE_DEPTH
// PER_VERTEX_COLOUR

/// <remarks>
/// <list type="bullet">
/// <item>
/// <term>PER_VERTEX_RADIUS</term>
/// <description>Enables the per-sphere radius being retrieved from the input
/// vertex. Otherwise, the global radius from the constant buffer will be used.
/// </description>
/// </item>
/// <item>
/// <term>PER_VERTEX_COLOUR</term>
/// <description>Enables the per-sphere colour being retrieved from the input
/// vertex. Otherwise, the global colour from the constant buffer will be used.
/// </description>
/// </item>
/// <item>
/// <term>PER_VERTEX_INTENSITY</term>
/// <description>A scalar intensity value is given for each vertex. This value
/// is either transformed to a colour directly in the vertex shader or passed
/// to the pixel shader to do the transfer function lookup. Note that
/// <c>PER_VERTEX_INTENSITY</c> and <c>PER_VERTEX_COLOUR</c> are mutually
/// exclusive.</description>
/// </item>
/// <item>
/// <term>PER_VERTEX_TRANSFER_FUNCTION</term>
/// <description>If defined, perform the transfer function lookup for the scalar
/// intensity given for each vertex directly in the vertex shader. Otherwise,
/// pass through the intensity to the pixel shader. This define is only
/// meaningful in combination with <c>PER_VERTEX_INTENSITY</c>.
/// </description>
/// </item>
/// <item>
/// <term>PARTICLES_FROM_BUFFER</term>
/// <description>If defined, read the properties of the particles from a
/// structured buffer view rather than from the vertex buffer. This technique is
/// used for all instancing-based rendering methods, which need to use the
/// vertex buffer to store geometry of the sprite/sphere/... to instance.
/// </description>
/// </item>
/// <item>
/// <term>CONVERT_COLOUR</term>
/// <description>If defined, the colour in the structured buffer view is stored
/// in 8-bit RGBA format and needs to be converted to float4 using bitwise
/// operations. This define is only meadningful in combination with
/// <c>PARTICLES_FROM_BUFFER</c>. For vertex buffers, all necessary conversions
/// are already done by Direct3D.
/// </description>
/// </item>
/// </list>
/// </remarks>

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
    float4 Position : POSITION;

#ifdef PER_VERTEX_COLOUR
    /// <summary>
    /// The colour of the particle.
    /// </summary>
    float4 Colour;

#elif PER_VERTEX_INTENSITY
    /// <summary>
    /// A scalar intensity which is transformed into a colour by means of
    /// a transfer function.
    /// </summary>
    float Intensity;
#endif /* PER_VERTEX_COLOUR */

#ifdef HOLOMOL
    /// <summary>
    /// The instance ID, which is required to determine the correct render
    /// target on HoloLens.
    /// </summary>
    uint Eye : SV_InstanceID;
#endif /* HOLOMOL */
};


/// <summary>
/// A particle from the structured buffer.
/// </summary>
struct Particle {
    /// <summary>
    /// The world-space position (xyz) and radius (w) of the particle.
    /// </summary>
    float4 Position;

#ifdef PER_VERTEX_COLOUR
    /// <summary>
    /// The colour of the particle.
    /// </summary>
#ifdef FLOAT_COLOUR
    float4 Colour;
#else /* FLOAT_COLOUR */
    uint Colour;
#endif /* FLOAT_COLOUR */

#elif PER_VERTEX_INTENSITY
    /// <summary>
    /// A scalar intensity which is transformed into a colour by means of
    /// a transfer function.
    /// </summary>
    float Intensity;
#endif /* PER_VERTEX_COLOUR */
};


/// <summary>
/// The output of the vertex shader, which goes, depending on the rendering
/// method, to the hull, geometry or pixel shader.
/// </summary>
struct VsOutput {
#if PER_VERTEX_TRANSFORM
    /// <summary>
    /// The transformed position of the vertex.
    /// </summary>
    float4 Position : POSITION;

    /// <summary>
    /// The original world-space position and radius of the particle.
    /// </summary>
    float4 SphereParams : TEXCOORD1;
#else /* PER_VERTEX_TRANSFORM */

    /// <summary>
    /// The original world-space position and radius of the particle.
    /// </summary>
    float4 SphereParams : POSITION;
#endif /* PER_VERTEX_TRANSFORM */

    /// <summary>
    /// The colour (or intensity without VS texture lookup) of the vertex.
    /// </summary>
    float4 Colour : COLOR;

#ifdef HOLOMOL
    uint Eye: TEXCOORD0;
#endif /* HOLOMOL */
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
    float4 Position : SV_POSITION;
    float4 Colour : COLOR0;
    float4 Normal : NORMAL0;
    nointerpolation float4 ViewDirection : TEXCOORD0;
#ifdef HOLOMOL
    nointerpolation uint Eye: SV_RenderTargetArrayIndex;
#endif /* HOLOMOL */
};


/// <summary>
/// The input of the pixel shader stage when performing raycasting of the
/// spheres.
/// </summary>
struct PsRaycastingInput {
#ifdef CONSERVATIVE_DEPTH
    linear noperspective sample float4 Position : SV_POSITION;
#else /* CONSERVATIVE_DEPTH */
    float4 Position : SV_POSITION;
#endif /* CONSERVATIVE_DEPTH */
    float4 SphereParams : TEXCOORD0;
    float4 Colour : COLOR0;
    nointerpolation float4 CameraPosition : TEXCOORD1;
    nointerpolation float4 CameraDirection : TEXCOORD2;
    nointerpolation float4 CameraUp : TEXCOORD3;
    nointerpolation float4 CameraRight : TEXCOORD4;
#ifdef HOLOMOL
    nointerpolation uint Eye: SV_RenderTargetArrayIndex;
#endif /* HOLOMOL */
};


/// <summary>
/// The output of the pixel shader stage.
/// </summary>
struct PsOutput {
    float4 Colour : SV_TARGET;
#ifdef CONSERVATIVE_DEPTH
    float Depth : SV_DepthGreaterEqual;
#else /* CONSERVATIVE_DEPTH */
    float Depth : SV_DEPTH;
#endif /* CONSERVATIVE_DEPTH */
};
#endif /* _MSC_VER */


#ifdef HOLOMOL
#define STEREO_BUFFERS (2)
#else /* HOLOMOL */
#define STEREO_BUFFERS (1)
#endif /* HOLOMOL */


cbuffer SphereConstants CBUFFER(0) {
    float4x4 ViewMatrix[STEREO_BUFFERS];
    float4x4 ProjMatrix[STEREO_BUFFERS];
    float4x4 ViewProjMatrix[STEREO_BUFFERS];
    float4x4 ViewInvMatrix[STEREO_BUFFERS];
    float4x4 ViewProjInvMatrix[STEREO_BUFFERS];
    float4 Viewport;
    float4 GlobalColour;
    float2 IntensityRange;
    float GlobalRadius;
    float TessellationFactor;
};
