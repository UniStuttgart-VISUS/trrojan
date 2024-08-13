// <copyright file="particle_properties.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
