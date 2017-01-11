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
        /// Answer the default factors to be tested if not specified by the
        /// user.
        /// </summary>
        /// <returns></returns>
        inline const trrojan::configuration_set& default_configs(void) const {
            return this->_default_configs;
        }

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
        std::vector<std::string> required_factors(void) const;

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

        virtual result run(const configuration& config) = 0;

        // TODO: define the interface.

    protected:

        inline benchmark_base(const std::string& name) : _name(name) { }

        inline benchmark_base(const std::string& name,
            trrojan::configuration_set default_configs)
            : _default_configs(default_configs), _name(name) { }

        /// <summary>
        /// Compares the given configuration to the last configuration and
        /// returns the names of the factors that have been changed.
        /// </summary>
        /// <remarks>
        /// The new configuration is directly saved (if
        /// <paramref name="update_last" /> is <c>true</c>) as reference for the
        /// next call to this function, ie the new call with the same parameter
        /// will yield no change.
        /// </remarks>
        /// <param name="new_config">The new configuration to be tested and
        /// stored as reference for the next call if
        /// <paramref name="update_last" /> is set.</param>
        /// <param name="oit">An output iterator for <see cref="std::string" />.
        /// </param>
        /// <param name="update_last">Controls whether the new configuration is
        /// remembered as last configuration (the default) or not.</param>
        /// <tparam name="I">The output iterator type.</tparam>
        /// <returns><c>true</c> if any factor has changed, <c>false</c>
        /// otherwise.</returns>
        template<class I>
        bool check_changed_factors(const trrojan::configuration& new_config,
            I oit, const bool update_last = true);

        /// <summary>
        /// Check whether the given configuration set has all required factors
        /// or raise an exception.
        /// </summary>
        void check_required_factors(const trrojan::configuration_set& cs) const;

        /// <summary>
        /// The default configuration set which is used to find required
        /// factors and to fill-in missing ones.
        /// </summary>
        /// <remarks>
        /// <para>It is safe to modfiy this while the benchmark is not being
        /// used.</para>
        /// <para>Any factor in this configuration set which does not have a
        /// manifestation is conisdered a required factor (the user must
        /// provide at least one manifestation for it).</para>
        /// </remarks>
        trrojan::configuration_set _default_configs;

    private:

        /// <summary>
        /// Allows for preserving the last configuration in order to determine
        /// which of the factors have changed.
        /// </summary>
        trrojan::configuration _last_config;

        /// <summary>
        /// The name of the benchmark.
        /// </summary>
        std::string _name;

    };

    /// <summary>
    /// A device.
    /// </summary>
    typedef std::shared_ptr<benchmark_base> benchmark;
}

#include "trrojan/benchmark.inl"
