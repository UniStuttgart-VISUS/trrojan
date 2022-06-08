// <copyright file="sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_benchmark.h"

#include "trrojan/log.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/stats_query.h"

#include "sphere_techniques.h"


/*
 * trrojan::d3d12::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d12::sphere_benchmark::sphere_benchmark(void)
    : sphere_benchmark_base("sphere-renderer") { }


/*
 * trrojan::d3d12::sphere_benchmark::on_device_switch
 */
void trrojan::d3d12::sphere_benchmark::on_device_switch(ID3D12Device *device) {
    assert(device != nullptr);
    sphere_benchmark_base::on_device_switch(device);

    // Make sure that we have one bundle allocator for the single bundle we are
    // recording to draw the spheres.
    this->_bundle_allocators.clear();
    create_command_allocators(this->_bundle_allocators, device,
        D3D12_COMMAND_LIST_TYPE_BUNDLE, 1);

    // Make sure that we have a descriptor heap with entries for CBVs and all
    // other SRVs.
    if (this->_descriptor_heaps.empty()) {
        this->create_descriptor_heap(device, 2);
    }
    assert(this->_descriptor_heaps.size() == this->pipeline_depth());
    assert(std::all_of(this->_descriptor_heaps.begin(),
        this->_descriptor_heaps.end(),
        [](const ATL::CComPtr<ID3D12DescriptorHeap>&p) { return p != nullptr; }));

    // Invalidate the data.
    this->_data = nullptr;
}


/*
 * trrojan::d3d12::sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::sphere_benchmark::on_run(d3d12::device& device,
        const configuration& config, const std::vector<std::string>& changed) {
    std::vector<gpu_timer::millis_type> bundle_times, gpu_times;
    std::uint32_t cpu_iterations = 1;
    timer cpu_timer;
    const auto gpu_freq = gpu_timer::get_timestamp_frequency(
        device.command_queue());
    const auto gpu_iterations = config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    gpu_timer gpu_timer(device.d3d_device(), 2, this->pipeline_depth());
    const auto min_wall_time = config.get<std::uint32_t>(factor_min_wall_time);
    stats_query::value_type pipeline_stats;
    auto shader_code = sphere_benchmark_base::get_shader_id(config);
    stats_query stats_query(device.d3d_device(), 1, 1);

    // If the data or any factor influencing their representation on the GPU
    // have changed, clear them.
    if (contains_any(changed, factor_data_set, factor_frame, factor_device,
            factor_force_float_colour, factor_fit_bounding_box)) {
        this->_data = nullptr;
    }

    // Load the data if necessary.
    if (this->_data == nullptr) {
        this->_data = this->load_data(device.d3d_device(), shader_code, config);
    }

    // Determine the number of primitives to emit and record the draw call
    // into a command bundle.
    auto pipeline = this->get_pipeline_state(device.d3d_device(), shader_code);
    auto root_sig = this->get_root_signature(device.d3d_device(), shader_code);
    auto bundle = this->create_command_bundle(0, pipeline);

    bundle->SetGraphicsRootSignature(root_sig);
    bundle->IASetPrimitiveTopology(get_primitive_topology(shader_code));
    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        // Instancing of quads requires 4 vertices per particle.
        bundle->DrawInstanced(4, this->get_sphere_count(), 0, 0);
    } else {
        bundle->DrawInstanced(this->get_sphere_count(), 1, 0, 0);
    }
    close_command_list(bundle);

    // Update constant buffers.
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        this->update_constants(config, i);
    }

    // Record a command list for each frame for the CPU measurements.
    std::vector<ATL::CComPtr<ID3D12GraphicsCommandList>> cmd_lists(
        this->pipeline_depth());
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        cmd_lists[i] = this->create_graphics_command_list(i);
        this->enable_target(cmd_lists[i], i);
        this->clear_target(cmd_lists[i], i);
        cmd_lists[i]->ExecuteBundle(bundle);
        this->disable_target(cmd_lists[i], i);
        close_command_list(cmd_lists[i]);
    }

    // Do prewarming and compute number of CPU iterations at the same time.
    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto batch_time = 0.0;
        auto prewarms = (std::max)(1u, config.get<std::uint32_t>(
            factor_min_prewarms));

        do {
            cpu_iterations = 0;
            assert(prewarms >= 1);

            cpu_timer.start();
            for (; cpu_iterations < prewarms; ++cpu_iterations) {
                auto cmd_list = cmd_lists[this->buffer_index()];
                device.execute_command_list(cmd_list);
                this->present_target();
            }
            device.wait_for_gpu();

            batch_time = cpu_timer.elapsed_millis();

            if (batch_time < min_wall_time) {
                prewarms = static_cast<std::uint32_t>(std::ceil(
                    (static_cast<double>(min_wall_time) * cpu_iterations)
                    / batch_time));
                if (prewarms < 1) {
                    prewarms = 1;
                }
            }
        } while (batch_time < min_wall_time);
    }

    // Do the wall clock measurement using the prepared command lists.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", cpu_iterations);
    cpu_timer.start();
    for (std::uint32_t i = 0; i < cpu_iterations; ++i) {
        auto cmd_list = cmd_lists[this->buffer_index()];
        device.execute_command_list(cmd_list);
        this->present_target();
    }
    device.wait_for_gpu();
    const auto cpu_time = cpu_timer.elapsed_millis();

    // Do the GPU counter measurements using individual command lists.
    bundle_times.resize(gpu_iterations);
    gpu_times.resize(gpu_iterations);
    for (std::uint32_t i = 0; i < gpu_iterations; ++i) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", i);
        auto cmd_list = cmd_lists[this->buffer_index()];
        this->reset_command_list(cmd_list);

        gpu_timer.start_frame();
        gpu_timer.start(cmd_list, 0);
        this->enable_target(cmd_list);
        this->clear_target(cmd_list);

        gpu_timer.start(cmd_list, 1);
        cmd_list->ExecuteBundle(bundle);
        gpu_timer.end(cmd_list, 1);

        this->disable_target(cmd_list);
        gpu_timer.end(cmd_list, 0);
        const auto timer_index = gpu_timer.end_frame(cmd_list);

        device.close_and_execute_command_list(cmd_list);
        this->present_target();

        gpu_times[i] = gpu_timer::to_milliseconds(
            gpu_timer.evaluate(timer_index, 0),
            gpu_freq);
        bundle_times[i] = gpu_timer::to_milliseconds(
            gpu_timer.evaluate(timer_index, 1),
            gpu_freq);
    }

    // Obtain pipeline statistics.
    log::instance().write_line(log_level::debug, "Collecting pipeline "
        "statistics ...");
    {
        auto cmd_list = cmd_lists[this->buffer_index()];
        this->reset_command_list(cmd_list);

        stats_query.begin_frame();

        this->enable_target(cmd_list);
        this->clear_target(cmd_list);

        stats_query.begin(cmd_list, 0);
        cmd_list->ExecuteBundle(bundle);
        stats_query.end(cmd_list, 0);

        this->disable_target(cmd_list);
        const auto stats_index = stats_query.end_frame(cmd_list);

        device.close_and_execute_command_list(cmd_list);
        this->present_target();

        pipeline_stats = stats_query.evaluate(stats_index, 0);
    }

    // Compute derived statistics for GPU counters.
    const auto bundle_median = calc_median(bundle_times);
    const auto gpu_median = calc_median(gpu_times);

    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(config,
        std::initializer_list<std::string> {
        "particles",
        "data_extents",
        "ia_vertices",
        "ia_primitives",
        "vs_invokes",
        "gs_invokes",
        "gs_primitives",
        "c_invokes",
        "c_primitives",
        "ps_invokes",
        "hs_invokes",
        "ds_invokes",
        "cs_invokes",
        "bundle_time_min",
        "bundle_time_med",
        "bundle_time_max",
        "gpu_time_min",
        "gpu_time_med",
        "gpu_time_max",
        "wall_time_iterations",
        "wall_time",
        "wall_time_avg"
    });

    // Output the results.
    retval->add({
        this->get_sphere_count(),
        this->get_data_extents(),
        pipeline_stats.IAVertices,
        pipeline_stats.IAPrimitives,
        pipeline_stats.VSInvocations,
        pipeline_stats.GSInvocations,
        pipeline_stats.GSPrimitives,
        pipeline_stats.CInvocations,
        pipeline_stats.CPrimitives,
        pipeline_stats.PSInvocations,
        pipeline_stats.HSInvocations,
        pipeline_stats.DSInvocations,
        pipeline_stats.CSInvocations,
        bundle_times.front(),
        bundle_median,
        bundle_times.back(),
        gpu_times.front(),
        gpu_median,
        gpu_times.back(),
        cpu_iterations,
        cpu_time,
        static_cast<double>(cpu_time) / cpu_iterations
        });

    // Store the result.

    return retval;
}
