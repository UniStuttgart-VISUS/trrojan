/// <copyright file="configuration.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/configuration.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "trrojan/system_factors.h"



/*
 * trrojan::configuration::add_system_factors
 */
void trrojan::configuration::add_system_factors(void) {
    system_factors::instance().get(std::back_inserter(this->_factors));
}


/*
 * trrojan::configuration::find
 */
trrojan::configuration::iterator_type trrojan::configuration::find(
        const std::string& factor) const {
    return std::find_if(this->_factors.cbegin(), this->_factors.cend(),
        [&factor](const named_variant& v) { return (v.name() == factor); });
}


/*
 * trrojan::configuration::add
 */
void trrojan::configuration::check_duplicate(const std::string& name) {
    if (this->contains(name)) {
        std::stringstream msg;
        msg << "The configuration already contains a factor named \""
            << name << "\"." << std::ends;
        throw std::invalid_argument(msg.str());
    }
}


/*
 * trrojan::to_string
 */
std::string trrojan::to_string(const configuration& c) {
    std::stringstream retval;
    retval << c << std::ends;
    return retval.str();
}
