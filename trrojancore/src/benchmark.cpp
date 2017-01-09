/// <copyright file="benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/benchmark.h"

#include <sstream>
#include <stdexcept>


/*
 * trrojan::benchmark_base::~benchmark_base
 */
trrojan::benchmark_base::~benchmark_base(void) { }


/*
 * trrojan::benchmark_base::required_factors
 */
std::vector<std::string> trrojan::benchmark_base::required_factors(void) const {
    std::vector<std::string> retval;

    for (auto& f : this->_default_configs.factors()) {
        if (f.size() == 0) {
            retval.push_back(f.name());
        }
    }

    return std::move(retval);
}


/*
 * trrojan::benchmark_base::check_required_factors
 */
void trrojan::benchmark_base::check_required_factors(
        const trrojan::configuration_set& cs) const {
    for (auto& f : this->_default_configs.factors()) {
        if ((f.size() == 0) && !cs.contains_factor(f.name())) {
            std::stringstream msg;
            msg << "The given configuration_set does not contain the required "
                "factor \"" << f.name() << "\"." << std::ends;
            throw std::invalid_argument(msg.str());
        }
    }
}
