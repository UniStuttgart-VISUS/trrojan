// <copyright file="staging_key.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <functional>
#include <string>

#include "trrojan/random_sphere_generator.h"

#include "trrojan/d3d12/sphere_data.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Uniquely identifies staged data sets used in
    /// <see cref="sphere_streaming_benchmark" /> and
    /// <see cref="dstorage_sphere_benchmark" />.
    /// </summary>
    class TRROJAND3D12_API staging_key final {

    public:

        staging_key(void) noexcept;

        std::uint32_t batch_size;
        std::uint32_t copies;
        std::string data_set;
        std::string folder;
        bool force_float;

        bool operator ==(const staging_key& rhs) const noexcept;

        inline bool operator !=(const staging_key& rhs) const noexcept {
            return !(*this == rhs);
        }

    };

} /* end namespace d3d12 */
} /* end namespace trrojan */


/// <summary>
/// Hashes the key identifying cached streaming files.
/// </summary>
template<> struct TRROJAND3D12_API std::hash<trrojan::d3d12::staging_key> {
    typedef trrojan::d3d12::staging_key value_type;
    std::size_t operator ()(const value_type& value) const noexcept;
};
