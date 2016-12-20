/// <copyright file="benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>

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

        virtual std::pair<configuration, result_set> run(
            const configuration_set& configs) = 0;

        // TODO: define the interface.

    protected:

        inline benchmark_base(const std::string& name) : _name(name) { }

    private:

        std::string _name;
    };

    /// <summary>
    /// A device.
    /// </summary>
    typedef std::shared_ptr<benchmark_base> benchmark;
}
