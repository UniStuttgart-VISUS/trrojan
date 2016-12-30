/// <copyright file="benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "trrojan/configuration_set.h"
#include "trrojan/export.h"
#include "trrojan/result_set.h"


namespace trrojan {

    /// <summary>
    /// Base class for all benchmarks.
    /// </summary>
    class TRROJANCORE_API benchmark_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~benchmark_base(void);

        /// <summary>
        /// Answer the name of the benchmark.
        /// </summary>
        /// <returns></returns>
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Answer the names of the factors which a
        /// <see cref="trrojan::configuration" /> passed to the benchmark must
        /// at least contain.
        /// </summary>
        /// <returns>The names of the factor which are required for the
        /// benchmark to run</returns>
        inline const std::vector<std::string>& required_factors(void) const {
            return this->_required_factors;
        }

        /// <summary>
        /// Run the benchmark for each of the
        /// <see cref="trrojan::configuration" />s of the given set and return
        /// the results.
        /// </summary>
        /// <param name="configs"></param>
        /// <returns></returns>
        /// <exception cref="std::invalid_argument">If not all required factors
        /// are specified in the configuration set.</exception>
        virtual result_set run(const configuration_set& configs) = 0;

        // TODO: define the interface.

    protected:

        inline benchmark_base(const std::string& name) : _name(name) { }

        inline benchmark_base(const std::string& name,
            std::initializer_list<std::string> required_factors)
            : _required_factors(required_factors), _name(name) { }

        /// <summary>
        /// Check whether the given configuration set has all required factors
        /// or raise an exception.
        /// </summary>
        void check_required_factors(const trrojan::configuration_set& cs) const;

        /// <summary>
        /// The list of factors the benchmark required to run.
        /// </summary>
        std::vector<std::string> _required_factors;

    private:

        std::string _name;
    };

    /// <summary>
    /// A device.
    /// </summary>
    typedef std::shared_ptr<benchmark_base> benchmark;
}
