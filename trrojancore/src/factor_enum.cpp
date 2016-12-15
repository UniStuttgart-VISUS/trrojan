/// <copyright file="factor_enum.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/factor_enum.h"



/*
 * trrojan::detail::factor_enum::~factor_enum
 */
trrojan::detail::factor_enum::~factor_enum(void) { }


/*
 * trrojan::detail::factor_enum::size
 */
size_t trrojan::detail::factor_enum::size(void) const {
    return this->manifestations.size();
}


/*
 * trrojan::detail::factor_enum::operator []
 */
const trrojan::variant& trrojan::detail::factor_enum::operator [](
        const size_t i) const {
    if (i >= this->size()) {
        throw std::range_error("The factor does not contain the requested "
            "manifestation.");
    }

    return this->manifestations[i];
}
