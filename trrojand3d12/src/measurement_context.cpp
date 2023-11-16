// <copyright file="measurement_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/measurement_context.h"

#include "trrojan/estimate_iterations.h"


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
    auto retval = estimate_iterations(min_wall_time, elapsed,
        this->cpu_iterations);

    if (retval > this->cpu_iterations) {
        return retval;
    } else {
        this->cpu_iterations = retval;
        return 0;
    }
}
