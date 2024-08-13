/// <copyright file="factor_enum.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/factor_enum.h"



/*
 * trrojan::detail::factor_enum::~factor_enum
 */
trrojan::detail::factor_enum::~factor_enum(void) { }


/*
 * trrojan::detail::factor_enum::clone
 */
std::unique_ptr<trrojan::detail::factor_base>
trrojan::detail::factor_enum::clone(void) const {
    return std::unique_ptr<factor_base>(
        new factor_enum(this->_name, this->manifestations));
}


/*
 * trrojan::detail::factor_enum::size
 */
size_t trrojan::detail::factor_enum::size(void) const {
    return this->manifestations.size();
}


/*
 * trrojan::detail::factor_enum::operator []
 */
trrojan::variant trrojan::detail::factor_enum::operator [](
        const size_t i) const {
    if (i >= this->size()) {
        throw std::range_error("The factor does not contain the requested "
            "manifestation.");
    }

    return this->manifestations[i];
}
