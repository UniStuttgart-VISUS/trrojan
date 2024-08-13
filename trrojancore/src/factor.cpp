/// <copyright file="factor.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/factor.h"

#include <cassert>
#include <stdexcept>


/*
 * trrojan::factor::factor
 */
trrojan::factor::factor(const factor& rhs) {
    if (rhs.impl != nullptr) {
        this->impl = rhs.impl->clone();
    }
}


/*
 * trrojan::factor::operator []
 */
trrojan::variant trrojan::factor::operator [](const size_t i) const {
    if (this->impl == nullptr) {
        throw std::range_error("The factor does not contain the requested "
            "manifestation.");
    }

    return (*this->impl)[i];
}


/*
 * trrojan::factor::operator =
 */
trrojan::factor& trrojan::factor::operator =(const factor & rhs) {
    if (this != std::addressof(rhs)) {
        this->impl = (rhs.impl) ? rhs.impl->clone() : nullptr;
    }
    return *this;
}


/*
 * trrojan::factor::operator =
 */
trrojan::factor& trrojan::factor::operator =(factor&& rhs) {
    if (this != std::addressof(rhs)) {
        this->impl = std::move(impl);
    }
    return *this;
}


/*
 * trrojan::factor::operator ==
 */
bool trrojan::factor::operator ==(const factor& rhs) const {
    return (((this->impl == nullptr) && (rhs.impl == nullptr))
        || ((this->impl != nullptr) && (rhs.impl != nullptr)
        && (*this->impl == *rhs.impl)));
}
