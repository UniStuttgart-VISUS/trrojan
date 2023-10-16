// <copyright file="measurement_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/measurement_context.h"


/*
 * trrojan::d3d12::measurement_context::measurement_context
 */
trrojan::d3d12::measurement_context::measurement_context(d3d12::device& device,
        const std::size_t gpu_ranges, const std::size_t pipeline_depth)
    : cpu_iterations(1),
        gpu_timer(device.d3d_device(), gpu_ranges, pipeline_depth),
        stats_query(device.d3d_device(), 1, 1) {
    this->_gpu_timer_frequency = gpu_timer::get_timestamp_frequency(
        device.command_queue());
}


/*
 * trrojan::d3d12::measurement_context::check_cpu_iterations
 */
std::uint32_t  trrojan::d3d12::measurement_context::check_cpu_iterations(
        const double min_wall_time) {
    const auto elapsed = cpu_timer.elapsed_millis();

    if (elapsed < min_wall_time) {
        // The elapsed time was insufficient, compute iterations for a new try.
        auto retval = static_cast<std::uint32_t>(std::ceil(
            (static_cast<double>(min_wall_time) * this->cpu_iterations)
            / elapsed));
        if (retval < 1) {
            retval = 1;
        }

        return retval;

    } else {
        // The given number of iterations was OK, signal not to try again.
        return 0;
    }
}
