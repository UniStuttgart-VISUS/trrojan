/// <copyright file="factor.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/factor.h"

#include <cassert>
#include <stdexcept>

#include "factor_base.h"


/*
 * trrojan::factor::size
 */
size_t trrojan::factor::size(void) const {
    return (this->impl != nullptr) ? this->impl->size() : 0;
}


/*
 * trrojan::factor::operator []
 */
const trrojan::variant& trrojan::factor::operator [](const size_t i) const {
    if (i >= this->size()) {
        throw std::range_error("The factor does not contain the requested "
            "manifestation.");
    }
    assert(this->impl != nullptr);

    return (*this->impl)[i];
}
