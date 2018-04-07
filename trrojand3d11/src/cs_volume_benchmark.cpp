/// <copyright file="cs_volume_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/cs_volume_benchmark.h"


/*
 * trrojan::d3d11::cs_volume_benchmark::cs_volume_benchmark
 */
trrojan::d3d11::cs_volume_benchmark::cs_volume_benchmark(void)
    : volume_benchmark_base("compute-volume-renderer") { }


/*
 * trrojan::d3d11::cs_volume_benchmark::on_run
 */
trrojan::result trrojan::d3d11::cs_volume_benchmark::on_run(
        d3d11::device& device, const configuration& config,
        const std::vector<std::string>& changed) {


    // Update the generic GPU resources.
    volume_benchmark_base::on_run(device, config, changed);

    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(std::move(config),
        std::initializer_list<std::string> {
            "data_extents",
            "gpu_time_min",
            "gpu_time_med",
            "gpu_time_max",
            "wall_time_iterations",
            "wall_time",
            "wall_time_avg"
    });


    return retval;
}
