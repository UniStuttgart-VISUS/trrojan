/// <copyright file="benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/benchmark.h"

#include <sstream>
#include <stdexcept>

#include "trrojan/log.h"
#include "trrojan/system_factors.h"


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
 * trrojan::benchmark_base::run
 */
trrojan::result_set trrojan::benchmark_base::run(
        const configuration_set& configs) {
    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto c = configs;
    c.merge(this->_default_configs, false);

    // Invoke each configuration.
    result_set retval;
    c.foreach_configuration([this, &retval](const configuration& c) {
        try {
            this->log_run(c);
            retval.push_back(std::move(this->run(c)));
            return true;
        } catch (const std::exception& ex) {
            log::instance().write_line(ex);
            return false;
        }
    });

    return retval;
}


/*
 * trrojan::benchmark_base::merge_system_factors
 */
trrojan::configuration& trrojan::benchmark_base::merge_system_factors(
        trrojan::configuration& c) {
    c.add_system_factors();
    return c;
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


/*
 * trrojan::benchmark_base::log_run
 */
void trrojan::benchmark_base::log_run(const trrojan::configuration& c) const {
    auto factors = trrojan::to_string(c);
    log::instance().write(log_level::information, "Running \"%s\" with %s\n",
        this->name().c_str(), factors.c_str());
}
