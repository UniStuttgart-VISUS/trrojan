/// <copyright file="benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#include "trrojan/benchmark.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

#include "trrojan/log.h"
#include "trrojan/system_factors.h"



/*
 * trrojan::benchmark_base::check_consistency
 */
void trrojan::benchmark_base::check_consistency(const result_set& rs) {
    if (rs.size() > 1) {
        /* Only a result set with more than one element can be inconsistent. */
        auto& reference = rs.front();
        if (reference == nullptr) {
            throw std::invalid_argument("A result_set must not contain nullptr "
                "elements.");
        }

        for (size_t i = 1; i < rs.size(); ++i) {
            auto& element = rs[i];
            if (element == nullptr) {
                throw std::invalid_argument("A result_set must not contain nullptr "
                    "elements.");
            }
            reference->check_consistency(*element);
        }
    }
}


/*
 * trrojan::benchmark_base::merge_results
 */
void trrojan::benchmark_base::merge_results(result_set& l,
        const result_set& r) {
    l.reserve(l.size() + r.size());
    l.insert(l.end(), r.cbegin(), r.cend());
}


/*
 * trrojan::benchmark_base::merge_results
 */
void trrojan::benchmark_base::merge_results(result_set& l, result_set&& r) {
    l.reserve(l.size() + r.size());
    for (auto& e : r) {
        l.push_back(std::move(e));
    }
}


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
size_t trrojan::benchmark_base::run(const configuration_set& configs,
        const on_result_callback& callback) {
    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto c = configs;
    c.merge(this->_default_configs, false);

    // Invoke each configuration.
    size_t retval = 0;
    c.foreach_configuration([this, &retval, &callback](const configuration& c) {
        try {
            this->log_run(c);
            auto r = callback(std::move(this->run(c)));
            ++retval;
            return r;
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
    auto fs = this->required_factors();
    for (auto& f : fs) {
        log::instance().write(log_level::verbose, "Checking availability of "
            "factor \"%s\" in the given configuration ...\n", f.c_str());
        if ((f.size() == 0) && !cs.contains_factor(f)) {
            std::stringstream msg;
            msg << "The given configuration_set does not contain the required "
                "factor \"" << f << "\"." << std::ends;
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
