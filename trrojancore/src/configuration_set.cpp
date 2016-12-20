/// <copyright file="configuration_set.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/configuration_set.h"

#include <stdexcept>


/*
 * trrojan::configuration_set::add_factor
 */
void trrojan::configuration_set::add_factor(const factor& factor) {
    if (factor.name().empty()) {
        throw std::invalid_argument("The name of a factor must not be empty.");
    }
    if (factor.size() < 1) {
        throw std::invalid_argument("A factor must have at least one "
            "manifestation.");
    }
    if (this->contains_factor(factor.name())) {
        throw std::invalid_argument("The configuration set already contains a "
            "factor with the given name.");
    }

    this->factors.push_back(factor);
}


/*
 * trrojan::configuration_set::foreach_configuration
 */
bool trrojan::configuration_set::foreach_configuration(
        std::function<bool(const configuration&)> cb) const {
    bool retval = true;

    if (!this->factors.empty() && cb) {
        size_t cntTests = 1;
        configuration config;
        std::vector<size_t> frequencies;

        config.reserve(this->factors.size());
        for (auto& f : this->factors) {
            frequencies.push_back(cntTests);
            cntTests *= f.size();
        }

        for (size_t i = 0; (i < cntTests) && retval; ++i) {
            config.clear();
            for (size_t j = 0; j < this->factors.size(); ++j) {
                auto ij = (i / frequencies[j]) % this->factors.size();
                config.emplace_back(this->factors[j].name(),
                    this->factors[j][ij]);
            }
            retval = cb(config);
        }
    } /* end if (!this->factors.empty()) */

    return retval;
}
