// <copyright file="staging_key.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/staging_key.h"


/*
 * trrojan::d3d12::staging_key::staging_key
 */
trrojan::d3d12::staging_key::staging_key(void) noexcept
    : batch_size(0), copies(0), force_float(false) { }


/*
 * trrojan::d3d12::staging_key::operator ==
 */
bool trrojan::d3d12::staging_key::operator ==(
        const staging_key& rhs) const noexcept {
    return (this->batch_size == rhs.batch_size)
        && (this->copies == rhs.copies)
        && (this->data_set == rhs.data_set)
        && (this->folder == rhs.folder)
        && (this->force_float == rhs.force_float);
}


/*
 * std::hash<trrojan::d3d12::staging_key>::operator ()
 */
std::size_t std::hash<trrojan::d3d12::staging_key>::operator ()(
        const value_type& value) const noexcept {
    static std::hash<std::string> string_hash;
    auto cnt = 0;

    auto retval = static_cast<std::size_t>(value.batch_size) << cnt++;
    retval ^= string_hash(value.data_set) << cnt++;
    retval ^= static_cast<std::size_t>(value.copies) << cnt++;
    retval ^= string_hash(value.folder) << cnt++;

    if (value.force_float) {
        retval = ~retval;
    }

    return retval;
}
