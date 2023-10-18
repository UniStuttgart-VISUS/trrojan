// <copyright file="random_sphere_data_set.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/configuration.h"
#include "trrojan/random_sphere_generator.h"

#include "trrojan/d3d11/sphere_data_set.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a random particle creator.
    /// </summary>
    class TRROJAND3D11_API random_sphere_data_set : public sphere_data_set_base {

    public:

        /// <summary>
        /// Possible types of spheres that can be created.
        /// </summary>
        typedef random_sphere_generator::sphere_type sphere_type;

        /// <summary>
        /// The type of flags controlling the data generation.
        /// </summary>
        typedef sphere_data_set_base::properties_type create_flags;

        typedef sphere_data_set_base::point_type point_type;
        typedef sphere_data_set_base::size_type size_type;

        /// <summary>
        /// Creates a data set of random spheres with the specified properties.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="flags">
        /// <see cref="sphere_data_set_base::property_structured_resource" /> to
        /// create a structured resource buffer, 0 for a vertex buffer.</param>
        /// <param name="sphereType"></param>
        /// <param name="cntParticles"></param>
        /// <param name="domainSize"></param>
        /// <param name="sphereSize"></param>
        /// <param name="seed"></param>
        /// <returns></returns>
        static sphere_data_set create(ID3D11Device *device,
            const create_flags flags,
            const sphere_type sphereType,
            const size_type cntParticles,
            const std::array<float, 3>& domainSize,
            const std::array<float, 2>& sphereSize,
            const std::uint32_t seed);

        /// <summary>
        /// Creates a data set of random spheres as described in the given
        /// <paramref name="configuration" /> string.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="flags">
        /// <see cref="sphere_data_set_base::property_structured_resource" /> to
        /// create a structured resource buffer, 0 for a vertex buffer. This
        /// value can be combined with
        /// <see cref="sphere_data_set_base::property_float_colour" /> to force
        /// colours using <c>float</c> channels instead of 8-bit.</param>
        /// <param name="configuration"></param>
        /// </returns></returns>
        static sphere_data_set create(ID3D11Device *device,
            const create_flags flags,
            const std::string& configuration);

        /// <summary>
        /// Creates a vector format descriptor for the random spheres.
        /// </summary>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> get_input_layout(
            const sphere_type type);

        /// <summary>
        /// Retrieves the data properties inherent to the type of spheres.
        /// </summary>
        static sphere_data_set_base::properties_type get_properties(
            const sphere_type type);

        /// <summary>
        /// Gets the stride (in bytes) of random spheres of the given type.
        /// </summary>
        inline static size_type get_stride(const sphere_type type) {
            auto retval = random_sphere_generator::get_stride(type);
            return static_cast<size_type>(retval);
        }

        virtual ~random_sphere_data_set(void) = default;

        /// <inheritdoc />
        virtual void bounding_box(point_type& outMin, point_type& outMax) const;

        /// <inheritdoc />
        virtual float max_radius(void) const;

        /// <summary>
        /// Recreates the data set using the given set of new flags.
        /// </summary>
        void recreate(ID3D11Device *device, const create_flags flags);

        /// <inheritdoc />
        virtual size_type size(void) const;

        /// <inheritdoc />
        virtual size_type stride(void) const;

    protected:

        static void minmax(point_type& i, point_type& a,
            const DirectX::XMFLOAT4& v);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        random_sphere_data_set(void);

        /// <summary>
        /// Stores the bounding box of the data set.
        /// </summary>
        point_type _bbox[2];

        /// <summary>
        /// Maximum radius of all spheres that have been generated.
        /// </summary>
        float _max_radius;

        /// <summary>
        /// Preserves the random seed that was used to generate the data.
        /// </summary>
        std::uint32_t _seed;

        /// <summary>
        /// Preserves the range of sphere sizes that was used to generate the
        /// data.
        /// </summary>
        std::array<float, 2> _sphere_size;

        /// <summary>
        /// The number of particles in the buffer.
        /// </summary>
        size_type _size;

        /// <summary>
        /// Type of last spheres that have been generated.
        /// </summary>
        sphere_type _type;

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
