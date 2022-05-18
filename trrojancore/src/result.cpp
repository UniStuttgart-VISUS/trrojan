// <copyright file="result.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Valentin Bruder</author>

#include "trrojan/result.h"

#include <algorithm>
#include <stdexcept>


/*
 * trrojan::basic_result::~basic_result
 */
trrojan::basic_result::~basic_result(void) { }


/*
 * trrojan::basic_result::check_consistency
 */
void trrojan::basic_result::check_consistency(const basic_result& other) const {
    if (this != std::addressof(other)) {
        this->_configuration.check_consistency(other.configuration());

        if (this->_result_names.size() != other._result_names.size()) {
            throw std::runtime_error("The result lists contain a different "
                "number of result names.");
        }
        for (auto& l : this->_result_names) {
            auto it = std::find(other._result_names.cbegin(),
                other._result_names.cend(), l);
            if (it == other._result_names.cend()) {
                throw std::runtime_error("The result lists contain different "
                    "types of results.");
            }
        }
    }
}


/*
 * trrojan::basic_result::results
 */
std::vector<std::vector<trrojan::named_variant>> trrojan::basic_result::results(
        void) const {
    std::vector<std::vector<named_variant>> retval;
    retval.reserve(this->measurements());

    for (size_t m = 0; m < this->measurements(); ++m) {
        retval.emplace_back();
        auto& r = retval.back();
        r.reserve(this->_result_names.size());

        for (size_t f = 0; f < this->_result_names.size(); ++f) {
            r[f] = named_variant(this->_result_names[f],
                this->_results[m * f + f]);
        }
    }

    return retval;
}


/*
 * trrojan::basic_result::results
 */
trrojan::basic_result::result_type trrojan::basic_result::results(
        const std::string& result_name) const {
    auto it = std::find(this->_result_names.cbegin(),
        this->_result_names.cend(), result_name);
    if (it == this->_result_names.cend()) {
        throw std::invalid_argument("The given result name was not found.");
    }
    auto d = std::distance(this->_result_names.cbegin(), it);

    result_type retval;
    retval.reserve(this->measurements());
    for (size_t i = d; i < this->_results.size(); i += d) {
        retval.push_back(this->_results[i]);
    }

    return retval;
}
