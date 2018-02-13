/// <copyright file="sphere_techniques.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdint>

#include "trrojan/particle_properties.h"


/// <summary>
/// This flag indicates that the technique requires a geometry shader.
/// </summary>
/// <remarks>
/// <para>The constants for the different techniques defined below include this
/// flag as necessary.</para>
/// </remarks>
#define SPHERE_TECHNIQUE_USE_GEO (static_cast<std::uint64_t>(1) << 31)

/// <summary>
/// This flag indicates that the technique requires a hull and a domain shader.
/// </summary>
/// <remarks>
/// <para>The constants for the different techniques defined below include this
/// flag as necessary.</para>
/// </remarks>
#define SPHERE_TECHNIQUE_USE_TESS (static_cast<std::uint64_t>(1) << 30)

/// <summary>
/// This flag indicates that the technique requires a structured resource view
/// for the particle data rather than a vertex buffer.
/// </summary>
#define SPHERE_TECHNIQUE_USE_SRV  (static_cast<std::uint64_t>(1) << 29)


#define __SPHERE_TECH_BASE (static_cast<std::uint64_t>(1) << 63)

/// <summary>
/// Raycasting on a quad created in the vertex shader by instancing.
/// </summary>
#define SPHERE_TECHNIQUE_QUAD_INST ((__SPHERE_TECH_BASE >> 0) | SPHERE_TECHNIQUE_USE_SRV)

/// <summary>
/// Raycasting on a polygon created in the vertex shader by instancing.
/// </summary>
#define SPHERE_TECHNIQUE_POLY_INST ((__SPHERE_TECH_BASE >> 1) | SPHERE_TECHNIQUE_USE_SRV)

/// <summary>
/// Raycasting on a quad created in the tessellation stage.
/// </summary>
#define SPHERE_TECHNIQUE_QUAD_TESS ((__SPHERE_TECH_BASE >> 2) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// Raycasting on a polygon created in the tessellation stage.
/// </summary>
#define SPHERE_TECHNIQUE_POLY_TESS ((__SPHERE_TECH_BASE >> 3) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// Raycasting on an adaptive polygon created in the tessellation stage.
/// </summary>
#define SPHERE_TECHNIQUE_ADAPT_POLY_TESS ((__SPHERE_TECH_BASE >> 4) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// MegaMol raycasting technique.
/// </summary>
#define SPHERE_TECHNIQUE_SPTA ((__SPHERE_TECH_BASE >> 5) | SPHERE_TECHNIQUE_USE_GEO)

/// <summary>
/// Raycasting on a quad created in the geometry shader.
/// </summary>
#define SPHERE_TECHNIQUE_GEO_QUAD ((__SPHERE_TECH_BASE >> 6) | SPHERE_TECHNIQUE_USE_GEO)

/// <summary>
/// Raycasting on a polygon created in the geometry shader.
/// </summary>
#define SPHERE_TECHNIQUE_GEO_POLY ((__SPHERE_TECH_BASE >> 7) | SPHERE_TECHNIQUE_USE_GEO)

/// <summary>
/// Tessellated sphere with fixed subdivision factors.
/// </summary>
#define SPHERE_TECHNIQUE_SPHERE_TESS ((__SPHERE_TECH_BASE >> 8) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// Tessellated sphere with adaptive subdivision.
/// </summary>
#define SPHERE_TECHNIQUE_ADAPT_SPHERE_TESS ((__SPHERE_TECH_BASE >> 9) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// Tessellated hemispheres with fixed subdivision factors.
/// </summary>
#define SPHERE_TECHNIQUE_HEMISPHERE_TESS ((__SPHERE_TECH_BASE >> 10) | SPHERE_TECHNIQUE_USE_TESS)

/// <summary>
/// Tessellated hemispheres with adaptive subdivision.
/// </summary>
#define SPHERE_TECHNIQUE_ADAPT_HEMISPHERE_TESS ((__SPHERE_TECH_BASE >> 11) | SPHERE_TECHNIQUE_USE_TESS)


/// <summary>
/// Each vertex has its own colour if this flag is set.
/// </summary>
#define SPHERE_INPUT_PV_COLOUR (static_cast<std::uint64_t>(trrojan::particle_properties::per_vertex_colour))

/// <summary>
/// Each vertex has its own radius if this flag is set.
/// </summary>
#define SPHERE_INPUT_PV_RADIUS (static_cast<std::uint64_t>(trrojan::particle_properties::per_vertex_radius))

/// <summary>
/// Each vertex has an intensity value which is transformed to a colour in the
/// vertex shader.
/// </summary>
#define SPHERE_INPUT_PV_INTENSITY (static_cast<std::uint64_t>(trrojan::particle_properties::per_vertex_intensity))

/// <summary>
/// Each vertex has an intensity value which is transformed to a colour in the
/// pixel shader.
/// </summary>
#define SPHERE_INPUT_PP_INTENSITY (static_cast<std::uint64_t>(1) << 16)

/// <summary>
/// The colour in structured data views is not RGB8, but converted to float4 on
/// the CPU.
/// </summary>
#define SPHERE_INPUT_FLT_COLOUR (static_cast<std::uint64_t>(1) << 17)
