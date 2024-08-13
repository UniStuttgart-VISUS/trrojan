/// <copyright file="cool_down.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <chrono>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Configuration of how the <see cref="executive" /> should insert
    /// cool-down periods during the benchmark.
    /// </summary>
    struct TRROJANCORE_API cool_down {

        /// <summary>
        /// The duration of a cool-down period in seconds.
        /// </summary>
        /// <remarks>
        /// If this value is zero, no cool-down periods are made.
        /// </remarks>
        std::chrono::seconds duration;

        /// <summary>
        /// The time between two cool-down periods in minutes.
        /// </summary>
        /// <remarks>
        /// If this value is zero, no cool-down periods are made.
        /// </remarks>
        std::chrono::minutes frequency;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline cool_down(void) : duration(std::chrono::seconds::zero()),
            frequency(std::chrono::minutes::zero()) { }

        /// <summary>
        /// Answer whether the cool-down configuration is enabled.
        /// </summary>
        /// <returns><c>true</c> if cool-down periods are enabled,
        /// <c>false</c> otherwise.</returns>
        inline bool enabled(void) const {
            return ((this->duration != std::chrono::seconds::zero())
                && (this->frequency != std::chrono::minutes::zero()));
        }
    };


    /// <summary>
    /// Utility class allowing benchmark implementors to easily perform
    /// cool-down checks.
    /// </summary>
    class TRROJANCORE_API cool_down_evaluator {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline cool_down_evaluator(const cool_down& config) : config(config),
            last(std::chrono::system_clock::now()) { }

        /// <summary>
        /// Check whether it is time for a cool-down and do so as necessary.
        /// </summary>
        void check(void);

    private:

        cool_down config;
        std::chrono::system_clock::time_point last;
    };
}
