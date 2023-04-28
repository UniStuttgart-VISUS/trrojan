// <copyright file="empty_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/empty_benchmark.h"

#include <array>

#include "trrojan/log.h"
#include "trrojan/result.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/utilities.h"


#define _EMPTY_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::empty_benchmark::factor_##f = #f

_EMPTY_BENCH_DEFINE_FACTOR(clear_colour);
_EMPTY_BENCH_DEFINE_FACTOR(cpu_counter_iterations);
_EMPTY_BENCH_DEFINE_FACTOR(gpu_counter_iterations);

#undef _EMPTY_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::empty_benchmark::empty_benchmark
 */
trrojan::d3d12::empty_benchmark::empty_benchmark(void) : benchmark_base("empty") {
    // Declare the configuration data we need to have.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_clear_colour, std::array<float, 4> { 0.0f, 0.0f, 0.0f, 0.0f}));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_cpu_counter_iterations, static_cast<std::uint32_t>(8)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_gpu_counter_iterations, static_cast<std::uint32_t>(8)));
}


/*
 * trrojan::d3d12::empty_benchmark::~empty_benchmark
 */
trrojan::d3d12::empty_benchmark::~empty_benchmark(void) { }


/*
 * trrojan::result trrojan::d3d12::empty_benchmark::on_run
 */
trrojan::result trrojan::d3d12::empty_benchmark::on_run(d3d12::device& device,
        const configuration& config, const std::vector<std::string>& changed) {
    auto clear_colour = config.get<std::array<float, 4>>(factor_clear_colour);
    const auto cpu_iterations = config.get<std::uint32_t>(
        factor_cpu_counter_iterations);
    timer cpu_timer;
    auto gpu_freq = gpu_timer::get_timestamp_frequency(device.command_queue());
    const auto gpu_iterations = config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    gpu_timer gpu_timer(device.d3d_device(), 2, this->pipeline_depth());
    std::vector<gpu_timer::millis_type> clear_times(gpu_iterations),
        gpu_times(gpu_iterations);

    for (std::uint32_t i = 0; i < gpu_iterations; ++i) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", i);
        auto cmd_list = this->create_graphics_command_list();

        gpu_timer.start_frame();
        gpu_timer.start(cmd_list, 0);
        this->enable_target(cmd_list);

        gpu_timer.start(cmd_list, 1);
        this->clear_target(clear_colour, cmd_list);
        gpu_timer.end(cmd_list, 1);

        this->disable_target(cmd_list);
        gpu_timer.end(cmd_list, 0);

        auto result_index = gpu_timer.end_frame(cmd_list);
        device.close_and_execute_command_list(cmd_list);
        this->present_target();

        gpu_times[i] = gpu_timer::to_milliseconds(
            gpu_timer.evaluate(result_index, 0),
            gpu_freq);
        clear_times[i] = gpu_timer::to_milliseconds(
            gpu_timer.evaluate(result_index, 1),
            gpu_freq);
    }

    // Record command lists for CPU measurement.
    std::vector<ATL::CComPtr<ID3D12GraphicsCommandList>> cmd_lists(
        this->pipeline_depth());
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        cmd_lists[i] = this->create_graphics_command_list(i);
        this->enable_target(cmd_lists[i], i);
        this->clear_target(clear_colour, cmd_lists[i], i);
        this->disable_target(cmd_lists[i], i);
        close_command_list(cmd_lists[i]);
    }

    log::instance().write_line(log_level::debug, "Performing {0} CPU "
        "measurement(s).", cpu_iterations);
    cpu_timer.start();

    for (std::uint32_t i = 0; i < cpu_iterations; ++i) {
        device.execute_command_list(cmd_lists[this->buffer_index()]);
        this->present_target();
    }

    const auto cpu_time = cpu_timer.elapsed_millis();
    const auto clear_med = calc_median(clear_times);
    const auto gpu_med = calc_median(gpu_times);


    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(config,
        std::initializer_list<std::string> {
            "clear_time_min",
            "clear_time_med",
            "clear_time_max",
            "gpu_time_min",
            "gpu_time_med",
            "gpu_time_max",
            "cpu_time_total",
            "cpu_time_mean",
        });

    // Collect the results.
    retval->add({
        clear_times.front(),
        clear_med,
        clear_times.back(),
        gpu_times.front(),
        gpu_med,
        gpu_times.back(),
        cpu_time,
        static_cast<double>(cpu_time) / cpu_iterations
    });

    return retval;
}
