/// <copyright file="particle_properties.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdint>


namespace trrojan {

    /// <summary>
    /// A bitmask which allows for identifying certain properties of a particle
    /// data set like the ones loaded by <see cref="trrojan::mmpld_reader" />
    /// </summary>
    enum particle_properties : std::uint32_t {
        /// <summary>
        /// There is a global colour and radius for all particles.
        /// </summary>
        none = 0,

        /// <summary>
        /// There is a separate colour for each particle.
        /// </summary>
        per_vertex_colour = 0x00000001,

        /// <summary>
        /// There is a separate radius for each particle.
        /// </summary>
        per_vertex_radius = 0x00000002,

        /// <summary>
        /// There is a separate intensity value per particle which needs to be
        /// transformed using a transfer function.
        /// </summary>
        per_vertex_intensity = 0x00000004
    };

}
