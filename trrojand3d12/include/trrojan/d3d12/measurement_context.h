// <copyright file="measurement_context.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/timer.h"

#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/stats_query.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Utility class managing all timers and pipeline queries.
    /// </summary>
    class TRROJAND3D12_API measurement_context final {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        measurement_context(d3d12::device& device, const std::size_t gpu_ranges,
            const std::size_t pipeline_depth);

        /// <summary>
        /// Check whether the elapsed wall time is more than the given minimum,
        /// and if so, commit the given number of iterations. Otherwise, return
        /// a new suggestion for the number of iterations that might be
        /// sufficient to reach the given minimum.
        /// </summary>
        std::uint32_t check_cpu_iterations(const double min_wall_time);

        /// <summary>
        /// The number of iterations 
        /// </summary>
        std::uint32_t cpu_iterations;

        /// <summary>
        /// The timer used for measuring wall-clock times.
        /// </summary>
        timer cpu_timer;

        /// <summary>
        /// The timer for measuring execution times on the GPU.
        /// </summary>
        gpu_timer gpu_timer;

        /// <summary>
        /// Helper for querying pipeline statistics.
        /// </summary>
        stats_query stats_query;

    private:

        gpu_timer::value_type _gpu_timer_frequency;

        //std::vector<gpu_timer::millis_type> bundle_times, gpu_times;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
