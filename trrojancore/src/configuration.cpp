/// <copyright file="configuration.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M?ller</author>

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
 * trrojan::configuration::check_consistency
 */
void trrojan::configuration::check_consistency(
        const configuration& other) const {
    if (this->_factors.size() != other._factors.size()) {
        throw std::runtime_error("The configurations contain a different "
            "number of factors.");
    }
    for (auto& l : this->_factors) {
        if (other.contains(l.name())) {
            throw std::runtime_error("The configurations contain different "
                "factors.");
        }
    }
}


/*
 * trrojan::configuration::contains
 */
bool trrojan::configuration::contains(const std::string& factor,
        const variant_type type) const {
    auto it = this->find(factor);
    if (it != this->_factors.end()) {
        return (it->value().type() == type);
    } else {
        return false;
    }
}


/*
 * trrojan::configuration::find
 */
trrojan::configuration::iterator_type trrojan::configuration::find(
        const std::string& factor) const {
    return std::find_if(this->_factors.cbegin(), this->_factors.cend(),
        [&factor](const named_variant& v) { return (v.name() == factor); });
}


///*
// * trrojan::configuration::replace
// */
//void trrojan::configuration::replace(const std::string& name,
//        const trrojan::variant& value) {
//    auto it = this->find0(name);
//    if (it != this->_factors.end()) {
//        *it = named_variant(name, value);
//    } else {
//        this->add(name, value);
//    }
//}
//
//
///*
// * trrojan::configuration::replace
// */
//void trrojan::configuration::replace(const std::string& name,
//        trrojan::variant&& value) {
//    auto it = this->find0(name);
//    if (it != this->_factors.end()) {
//        *it = named_variant(name, std::move(value));
//    } else {
//        this->add(name, std::move(value));
//    }
//}


/*
 * trrojan::to_string
 */
std::string trrojan::to_string(const configuration& c) {
    std::stringstream retval;
    retval << c << std::ends;
    return retval.str();
}



/*
 * trrojan::configuration::check_duplicate
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
 * trrojan::configuration::find0
 */
trrojan::configuration::container_type::iterator trrojan::configuration::find0(
        const std::string& factor) {
    return std::find_if(this->_factors.begin(), this->_factors.end(),
        [&factor](const named_variant& v) { return (v.name() == factor); });
}
