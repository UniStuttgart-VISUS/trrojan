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
    /// Checks whether <paramref name="actual_time" /> is
    /// <paramramref name="target_time" /> or more and updates the number of
    /// iterations based on the assumtion that
    /// <paramref name="actual_iterations" /> took
    /// <paramref name="actual_time" />.
    /// </summary>
    /// <param name="target_time"></param>
    /// <param name="actual_time"></param>
    /// <param name="actual_iterations"></param>
    /// <param name="precision"></param>
    /// <returns>The number of iterations to retry with or zero if the actual
    /// number of iterations did suffice.</returns>
    std::uint32_t TRROJANCORE_API estimate_iterations(const double target_time,
        const double actual_time, const std::uint32_t actual_iterations,
        const float precision);

} /* namespace trrojan */
