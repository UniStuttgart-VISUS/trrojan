// <copyright file="estimate_iterations.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/estimate_iterations.h"

#include <algorithm>
#include <cassert>
#include <cmath>


/*
 * trrojan::estimate_iterations
 */
std::uint32_t trrojan::estimate_iterations(const double target_time,
        const double actual_time, const std::uint32_t actual_iterations) {
    // t / (a / i) => t * i / a
    auto retval = static_cast<std::uint32_t>(std::ceil(
        target_time * actual_iterations / actual_time));

    if (retval < 1) {
        retval = 1;
    }

    return retval;
}
