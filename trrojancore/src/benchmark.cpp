// <copyright file="benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Valentin Bruder</author>
// <author>Christoph Müller</author>

#include "trrojan/benchmark.h"

#include <algorithm>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include "trrojan/com_error_category.h"
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
 * trrojan::benchmark_base::factor_device
 */
const std::string trrojan::benchmark_base::factor_device("device");


/*
 * trrojan::benchmark_base::factor_environment
 */
const std::string trrojan::benchmark_base::factor_environment("environment");


/*
 * trrojan::benchmark_base::~benchmark_base
 */
trrojan::benchmark_base::~benchmark_base(void) { }


/*
 * trrojan::benchmark_base::can_run
 */
bool trrojan::benchmark_base::can_run(environment env,
        device device) const {
    return true;
}


/*
 * trrojan::benchmark_base::optimise_order
 */
void trrojan::benchmark_base::optimise_order(configuration_set& inOutConfs) { }


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
        const on_result_callback& resultCallback,
        const cool_down& coolDown,
        const std::size_t continue_at) {
    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto c = configs;
    c.merge(this->_default_configs, false);

    // Invoke each configuration.
    cool_down_evaluator cde(coolDown);
    size_t retval = 0;
    c.foreach_configuration([&](configuration& c) -> bool {
        try {
            auto e = c.get<trrojan::environment>(environment_base::factor_name);
            auto d = c.get<trrojan::device>(device_base::factor_name);

            cde.check();

            if (this->can_run(e, d)) {
                if (retval >= continue_at) {
                    c.add_system_factors();
                    this->log_run(c);
                    auto r = resultCallback(std::move(this->run(c)));
                }
                ++retval;
                log::instance().write_line(log_level::information, "Completed "
                    "configuration #{0}. ", retval);
                return true;
            } else {
                log::instance().write_line(log_level::information, "A "
                    "benchmark cannot run with the specified combination of "
                    "environment and device. Skipping it ...");
                return true;
            }

        } catch (const std::system_error &ex) {
            log::instance().write_line(log_level::error, "An unexpected system "
                "error 0x{0:x} ({1}) was encountered while running a "
                "benchmark.", ex.code().value(), ex.what());
            return false;

        } catch (const std::exception& ex) {
            log::instance().write_line(ex);
            return false;

        } catch (...) {
            log::instance().write_line(log_level::error, "An unexpected "
                "exception was encountered while running a benchmark.");
            return false;
        }
    });

    log::instance().write_line(log_level::information, "Completed benchmarking "
        "of {0} individual configuration(s). ", retval);
    return retval;
}


/*
 * trrojan::benchmark_base::enter_power_scope
 */
std::string trrojan::benchmark_base::enter_power_scope(
        const power_collector::pointer& collector) {
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    if (powerCollector != nullptr) {
        // If we have a power sensor, we want to record data now.
        powerUid = powerCollector->set_next_unique_description();
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

    return "";
}


/*
 * trrojan::benchmark_base::initialise_power_collector
 */
trrojan::power_collector::pointer
trrojan::benchmark_base::initialise_power_collector(
        const trrojan::configuration& c) {
    power_collector::pointer retval;

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    auto it = c.find(power_collector::factor_name);
    if (it != c.end()) {
        retval = it->value().as<power_collector::pointer>();
    }

    if (retval != nullptr) {
        retval->set_header();
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

    return retval;
}


/*
 * trrojan::benchmark_base::leave_power_scope
 */
void trrojan::benchmark_base::leave_power_scope(
        const power_collector::pointer& collector) {
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    if (powerCollector != nullptr) {
        powerCollector->set_description("");
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
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
            "factor \"{}\" in the given configuration ...\n", f.c_str());
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
    log::instance().write(log_level::information, "Running \"{}\" with {}\n",
        this->name().c_str(), factors.c_str());
}
