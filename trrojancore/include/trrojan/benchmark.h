// <copyright file="benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Valentin Bruder</author>
// <author>Christoph Müller</author>

#pragma once

#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "trrojan/configuration_set.h"
#include "trrojan/contains.h"
#include "trrojan/cool_down.h"
#include "trrojan/device.h"
#include "trrojan/environment.h"
#include "trrojan/export.h"
#include "trrojan/power_collector.h"
#include "trrojan/result_set.h"


namespace trrojan {

    /// <summary>
    /// Base class for all benchmarks.
    /// </summary>
    class TRROJANCORE_API benchmark_base {

    public:

        /// <summary>
        /// A callback which is invoked after each run.
        /// </summary>
        typedef std::function<bool(result&&)> on_result_callback;

        /// <summary>
        /// Ensure that all results in <paramref name="rs" /> have the same,
        /// consistent content.
        /// </summary>
        /// <param name="rs"></param>
        /// <exception cref="std::runtime_error">In case there are
        /// inconsistencies in the result set.</exception>
        static void check_consistency(const result_set& rs);

        /// <summary>
        /// Merge the results from <paramref name="r" /> into
        /// <paramref name="l" />.
        /// </summary>
        static void merge_results(result_set& l, const result_set& r);

        /// <summary>
        /// Move the results from <paramref name="r" /> into
        /// <paramref name="l" />.
        /// </summary>
        static void merge_results(result_set& l, result_set&& r);

        /// <summary>
        /// The string &quot;device&quot; for identifying a
        /// <see cref="trrojan::device" /> as factor.
        /// </summary>
        /// <remarks>
        /// Bechmarks should use this constant to make sure that built-in
        /// functionality referencing devices is working as intended.
        /// </remarks>
        static const std::string factor_device;

        /// <summary>
        /// The string &quot;device&quot; for identifying an
        /// <see cref="trrojan::environment" /> as factor.
        /// </summary>
        /// <remarks>
        /// Bechmarks should use this constant to make sure that built-in
        /// functionality referencing environments is working as intended.
        /// </remarks>
        static const std::string factor_environment;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~benchmark_base(void);

        /// <summary>
        /// Answer whether the benchmark can run under the given environment on
        /// the given device.
        /// </summary>
        /// <remarks>
        /// <para>Subclasses should override this method to implement
        /// restrictions on the environment and devices they can run with.
        /// </para>
        /// <para>The default implementation of the <see cref="run" /> method
        /// uses this information to skip unsupported configurations.</para>
        /// </remarks>
        /// <param name="env">A benchmarking environment to be tested.</param>
        /// <param name="device">A device to be tested.</param>
        /// <returns><c>true</c>, unconditionally.</returns>
        virtual bool can_run(environment env, device device) const;

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
        /// Optimises the order of configuration factors such that the overhead
        /// of switching them is minimal for the benchmark.
        /// </summary>
        /// <remarks>
        /// The default implementation does nothing.
        /// </remarks>
        /// <param name="inOutConfs">The configuration set to be optimised.
        /// </param>
        virtual void optimise_order(configuration_set& inOutConfs);

        /// <summary>
        /// Answer the names of the factors which a
        /// <see cref="trrojan::configuration" /> passed to the benchmark must
        /// at least contain.
        /// </summary>
        /// <remarks>
        /// This implementation returns all factors specified in the default
        /// configurations. Subclasses might want to change this behaviour by
        /// overriding this method.
        /// </remarks>
        /// <returns>The names of the factor which are required for the
        /// benchmark to run</returns>
        virtual std::vector<std::string> required_factors(void) const;

        /// <summary>
        /// Run the benchmark for each of the
        /// <see cref="trrojan::configuration" />s of the given set and return
        /// the results to the given callback.
        /// </summary>
        /// <remarks>
        /// The base class provides a general implementation executing all
        /// configurations in <paramref name="configs" /> by enumerating them
        /// and calling the pure virtual <see cref="run" /> method. Implementing
        /// subclasses may overwrite this implementation with a more efficient
        /// one. This implementation should honour the cool-down periods
        /// requested in <paramref name="coolDown" />. The
        /// <see cref="trrojan::cool_down_evaluator" /> is a RAII-inspired
        /// utility class for implementing the requested cool-down periods.
        /// </remarks>
        /// <param name="configs"></param>
        /// <param name="resultCallback"></param>
        /// <param name="coolDown"></param>
        /// <returns>The total number of <see cref="trrojan::result" />s that
        /// have been returned to <paramref name="callback" />.</returns>
        /// <exception cref="std::invalid_argument">If not all required factors
        /// are specified in the configuration set.</exception>
        virtual size_t run(const configuration_set& configs,
            const on_result_callback& resultCallback,
            const cool_down& coolDown,
            const std::size_t continue_at);

        virtual result run(const configuration& config) = 0;

        // TODO: define the interface.

    protected:

        /// <summary>
        /// If <paramref name="collector" /> is not <c>nullptr</c>, enter a new
        /// unique power measurement scope and return its name.
        /// </summary>
        /// <param name="collector">An optional power collector.</param>
        /// <returns>The ID of the power measuring scope.</returns>
        static std::string enter_power_scope(
            const power_collector::pointer& collector);

        /// <summary>
        /// Checks whether <paramref name="c" /> contains a power collector, and
        /// if so, sets the output header.
        /// </summary>
        /// <param name="c">The configuration to retrieve the collector from.
        /// </param>
        /// <returns>The power collector if there was one and it has been
        /// successfully initialised, <c>nullptr</c> otherwise.</returns>
        static power_collector::pointer initialise_power_collector(
            const trrojan::configuration& c);

        /// <summary>
        /// If <paramref name="collector" /> is not <c>nullptr</c>, notify it
        /// that the active measurement scope was left. The collector will
        /// commit all power samples collected since
        /// <see cref="enter_power_scope" /> and prevent collection of further
        /// samples until the next scope is entered.
        /// </summary>
        /// <param name="collector">An optional power collector.</param>
        static void leave_power_scope(
            const power_collector::pointer& collector);

        /// <summary>
        /// Merges all system factors into <paramref name="c" />.
        /// </summary>
        static trrojan::configuration& merge_system_factors(
            trrojan::configuration& c);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the benchmark, which must be unique
        /// within its plugin.</param>
        inline benchmark_base(const std::string& name) : _name(name) { }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name">The name of the benchmark, which must be unique
        /// within its plugin.</param>
        /// <param name="default_configs">The default configurations to be
        /// tested.</param>
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
        /// <remarks>
        /// This implementation considers the factors provided by the
        /// <see cref="required_factors" /> method. Subclasses can change this 
        /// behaviour by overriding this method.
        /// </remarks>
        virtual void check_required_factors(
            const trrojan::configuration_set& cs) const;

        /// <summary>
        /// Write an informational message to the log that we are now running
        /// <paramref name="c" />.
        /// </summary>
        void log_run(const trrojan::configuration& c) const;

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
