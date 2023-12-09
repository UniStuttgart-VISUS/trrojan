// <copyright file="random_sphere_generator.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <cinttypes>
#include <vector>

#include <mmpld.h>

#include "trrojan/configuration.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Utility class for generating random sphere data sets in a predictable
    /// manner.
    /// </summary>
    class TRROJANCORE_API random_sphere_generator final {

    public:

        /// <summary>
        /// Details the how the particles are created.
        /// </summary>
        //enum class create_flags : std::uint32_t {
        //    per_particle_colour = static_cast<std::uint32_t>(mmpld::particle_properties::per_particle_colour),
        //    per_particle_radius = static_cast<std::uint32_t>(mmpld::particle_properties::per_particle_radius),
        //    per_particle_intensity = static_cast<std::uint32_t>(mmpld::particle_properties::per_particle_intensity),
        //    float_colour = static_cast<std::uint32_t>(mmpld::particle_properties::float_colour),
        //};
        typedef mmpld::particle_properties create_flags;

        /// <summary>
        /// A bit field to store the properties of the generated particles.
        /// </summary>
        typedef mmpld::particle_properties properties_type;

        /// <summary>
        /// Possible types of spheres that can be created.
        /// </summary>
        enum class sphere_type {
            unspecified,
            pos_intensity,
            pos_rgba32,
            pos_rgba8,
            pos_rad_intensity,
            pos_rad_rgba32,
            pos_rad_rgba8
        };

        /// <summary>
        /// The description of a random sphere data set.
        /// </summary>
        struct description {
            std::array<float, 3> domain_size;
            create_flags flags;
            std::size_t number;
            std::uint32_t seed;
            std::array<float, 2> sphere_size;
            sphere_type type;

            inline description(void)
                : domain_size({ 0.0f, 0.0f, 0.0f }),
                flags(static_cast<create_flags>(0)),
                number(0),
                seed(0),
                sphere_size({ 0.0f, 0.0f }),
                type(sphere_type::unspecified) { }
        };

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" /> and stores the particle to
        /// <paramref name="dst" />.
        /// </summary>
        /// <param name="dst">The destination buffer. This parameter can be
        /// <c>nullptr</c> for measuring the required buffer size.</param>
        /// <param name="cnt_bytes">The size of <paramref name="dst" /> in
        /// bytes.</param>
        /// <param name="out_max_radius">Receives the maximum radius of all
        /// generated spheres.</param>
        /// <param name="description">The description of the data to be created.
        /// </param>
        /// <returns>The number of bytes written to the output buffer.</returns>
        static std::size_t create(void *dst, const std::size_t cnt_bytes,
            float& out_max_radius, const description& description);

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" /> and stores the particle to
        /// <paramref name="dst" />.
        /// </summary>
        /// <param name="dst">The destination buffer. This parameter can be
        /// <c>nullptr</c> for measuring the required buffer size.</param>
        /// <param name="cnt_bytes">The size of <paramref name="dst" /> in
        /// bytes.</param>
        /// <param name="description">The description of the data to be created.
        /// </param>
        /// <returns>The number of bytes written to the output buffer.</returns>
        static inline std::size_t create(void *dst, const std::size_t cnt_bytes,
                const description& description) {
            float dummy;
            return create(dst, cnt_bytes, dummy, description);
        }

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" />.
        /// </summary>
        /// <param name="description">The description of the data to be
        /// created.</param>
        /// <returns>A buffer holding the data.</returns>
        static std::vector<std::uint8_t> create(const description& description);

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" />.
        /// </summary>
        /// <param name="out_max_radius">Receives the maximum radius of all
        /// generated spheres.</param>
        /// <param name="description">The description of the data to be
        /// created.</param>
        /// <returns>A buffer holding the data.</returns>
        static std::vector<std::uint8_t> create(float& out_max_radius,
            const description &description);

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" /> and stores the particle to
        /// <paramref name="dst" />.
        /// </summary>
        /// <param name="dst">The destination buffer. This parameter can be
        /// <c>nullptr</c> for measuring the required buffer size.</param>
        /// <param name="cnt_bytes">The size of <paramref name="dst" /> in
        /// bytes.</param>
        /// <param name="description">The description of the data to be created.
        /// </param>
        /// <param name="flags">Additional creation flags that are used when
        /// parsing the description. At the moment, this only supports
        /// <see cref="create_flags::float_colour" /> which will force colour
        /// to be represented as floating-point number.</param>
        /// <returns>The number of bytes written to the output buffer.</returns>
        static std::size_t create(void *dst, const std::size_t cnt_bytes,
            const std::string& description,
            const create_flags flags = create_flags::none);

        /// <summary>
        /// Creates a data set according to the given
        /// <paramref name="description" />.
        /// </summary>
        /// <param name="description">The description of the data to be
        /// created.</param>
        /// <param name="flags">Additional creation flags that are used when
        /// parsing the description. At the moment, this only supports
        /// <see cref="create_flags::float_colour" /> which will force colour
        /// to be represented as floating-point number.</param>
        /// <returns>A buffer holding the data.</returns>
        static std::vector<std::uint8_t> create(const std::string& description,
            const create_flags flags = create_flags::none);

        /// <summary>
        /// Creates a file name for caching random spheres generated by the given
        /// <paramref name="description" />.
        /// </summary>
        /// <param name="description"></param>
        /// <param name="folder"></param>
        /// <param name="prefix"></param>
        /// <param name="suffix"></param>
        /// <param name="extension"></param>
        /// <returns></returns>
        static std::string get_file_name(const description& description,
            const std::string& folder,
            const std::string& prefix = "",
            const std::string& suffix = "",
            const std::string& extension = ".rndsphcache");

        /// <summary>
        /// Generates the D3D11 or D3D12 input layout for the given type of
        /// sphere.
        /// </summary>
        /// <typeparam name="TInputElement"></typeparam>
        /// <param name="type"></param>
        /// <returns></returns>
        template<class TInputElement>
        static std::vector<TInputElement> get_input_layout(
            const sphere_type type);

        /// <summary>
        /// Retrieves the data properties inherent to the type of spheres.
        /// </summary>
        static properties_type get_properties(const sphere_type type);

        /// <summary>
        /// Computes the stride (in bytes) of random spheres of the given type.
        /// </summary>
        static std::size_t get_stride(const sphere_type type);

        /// <summary>
        /// Parses the textual description of random spheres in TRROLL scripts.
        /// </summary>
        /// <param name="description"></param>
        /// <param name="flags"></param>
        /// <returns></returns>
        static description parse_description(const std::string& description,
            const create_flags flags = create_flags::none);

        /// <summary>
        /// Converts a <see cref="description" /> object back to the string form
        /// used in TRROLL scripts.
        /// </summary>
        /// <param name="description"></param>
        /// <returns></returns>
        static std::string to_string(const description& description);
    };

} /* namespace trrojan */

#include "trrojan/random_sphere_generator.inl"
