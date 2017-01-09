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
 * trrojan::benchmark_base::check_required_factors
 */
void trrojan::benchmark_base::check_required_factors(
        const trrojan::configuration_set& cs) const {
    for (auto& r : this->_required_factors) {
        if (!cs.contains_factor(r)) {
            std::stringstream msg;
            msg << "The given configuration_set does not contain the required "
                "factor \"" << r << "\"." << std::ends;
            throw std::invalid_argument(msg.str());
        }
    }
}
