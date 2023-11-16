// <copyright file="estimate_iterations.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <utility>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Determines, based on <paramref name="actual_iterations" /> taking
    /// <paramref name="actual_time" />, how many iterations should be performed
    /// to reach <paramref name="target_time" />.
    /// </summary>
    /// <param name="target_time"></param>
    /// <param name="actual_time"></param>
    /// <param name="actual_iterations"></param>
    /// <returns>The number of iterations to run, which is at least 1.</returns>
    std::uint32_t TRROJANCORE_API estimate_iterations(const double target_time,
        const double actual_time, const std::uint32_t actual_iterations);

} /* namespace trrojan */
