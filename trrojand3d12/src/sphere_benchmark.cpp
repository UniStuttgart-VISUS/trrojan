// <copyright file="sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_benchmark.h"

#include "trrojan/log.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/gpu_timer.h"
#include "trrojan/d3d12/measurement_context.h"
#include "trrojan/d3d12/stats_query.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"



/*
 * trrojan::d3d12::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d12::sphere_benchmark::sphere_benchmark(void)
    : sphere_benchmark_base("sphere-renderer") { }


/*
 * trrojan::d3d12::sphere_benchmark::on_device_switch
 */
void trrojan::d3d12::sphere_benchmark::on_device_switch(device& device) {
    assert(device.d3d_device() != nullptr);
    sphere_benchmark_base::on_device_switch(device);

    // Make sure that we have one bundle allocator for the single bundle we are
    // recording to draw the spheres.
    this->_bundle_allocators.clear();
    create_command_allocators(this->_bundle_allocators, device.d3d_device(),
        D3D12_COMMAND_LIST_TYPE_BUNDLE, 1);
}


/*
 * trrojan::d3d12::sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::sphere_benchmark::on_run(d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    sphere_rendering_configuration cfg(config);
    std::vector<gpu_timer::millis_type> bundle_times, gpu_times;
    const auto gpu_freq = gpu_timer::get_timestamp_frequency(
        device.command_queue());
    measurement_context mctx(device, 2, this->pipeline_depth());
    stats_query::value_type pipeline_stats;
    auto shader_code = cfg.shader_id();
    stats_query stats_query(device.d3d_device(), 1, 1);

    // If the data or any factor influencing their representation on the GPU
    // have changed, clear them.
    this->clear_stale_data(changed);

    // Load the data if necessary.
    if (!this->_data) {
        log::instance().write_line(log_level::information, "Loading data set \""
            "{}\" ...", cfg.data_set());
        auto cmd_list = this->create_graphics_command_list();
        auto upload = this->_data.load(cmd_list, shader_code, cfg,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
            | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        device.close_and_execute_command_list(cmd_list);

        log::instance().write_line(log_level::verbose, "Waiting for data "
            "set to be loaded to the GPU ...");
        device.wait_for_gpu();
    }

    // Now that we have the data, update the shader code to match it.
    shader_code = this->_data.adjust_shader_code(shader_code);

    // As we have the data, we can also configure the camera now.
    this->configure_camera(config);

    // Prepare the descriptor heaps required for the requested technique and
    // data set. Note that create_descriptor_heap will access the data
    // properties, so this must be done *after* loading the data.
    this->create_descriptor_heaps(device.d3d_device(), shader_code);

    // Determine the number of primitives to emit and record the draw call
    // into a command bundle.
    auto pipeline = this->get_pipeline_state(device.d3d_device(), shader_code); 
    auto root_sig = this->get_root_signature(device.d3d_device(), shader_code);
    auto bundle = this->create_command_bundle(0, pipeline);
    set_debug_object_name(bundle, "Drawing bundle of sphere_benchmark");
    // Note: we always use the first descriptor heap as nothing changes from
    // frame to frame in one run of the benchmark.
    auto desc_tables = this->set_descriptors(device.d3d_device(), shader_code,
        0);

    bundle->SetGraphicsRootSignature(root_sig);
    bundle->SetPipelineState(pipeline);
    bundle->IASetPrimitiveTopology(get_primitive_topology(shader_code));
    set_descriptors(bundle, desc_tables);
    this->set_vertex_buffer(bundle, shader_code);

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        // Instancing of quads requires 4 vertices per particle.
        log::instance().write_line(log_level::debug, "Drawing {0} instances of "
            "four vertices.", this->_data.spheres());
        bundle->DrawInstanced(4, this->_data.spheres(), 0, 0);
    } else {
        log::instance().write_line(log_level::debug, "Drawing 1 instance of "
            "{0} vertices.", this->_data.spheres());
        bundle->DrawInstanced(this->_data.spheres(), 1, 0, 0);
    }
    close_command_list(bundle);

    // Update constant buffers.
    this->update_constants(cfg, 0);

    // Record a command list for each frame for the CPU measurements.
    std::vector<ATL::CComPtr<ID3D12GraphicsCommandList>> cmd_lists(
        this->pipeline_depth());
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        auto cmd_list = cmd_lists[i] = this->create_graphics_command_list(i);
        set_debug_object_name(cmd_list, "CPU command list #{}", i);

        cmd_list->SetGraphicsRootSignature(root_sig);
        cmd_list->SetPipelineState(pipeline);
        cmd_list->IASetPrimitiveTopology(get_primitive_topology(shader_code));
        this->set_descriptors(cmd_list, desc_tables);
        this->set_vertex_buffer(cmd_list, shader_code);

        this->enable_target(cmd_list, i);
        this->clear_target(cmd_list, i);

        //cmd_list->ExecuteBundle(bundle);
        if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
            // Instancing of quads requires 4 vertices per particle.
            log::instance().write_line(log_level::debug, "Drawing {0} instances of "
                "four vertices.", this->_data.spheres());
            cmd_list->DrawInstanced(4, this->_data.spheres(), 0, 0);
        } else {
            log::instance().write_line(log_level::debug, "Drawing 1 instance of "
                "{0} vertices.", this->_data.spheres());
            cmd_list->DrawInstanced(this->_data.spheres(), 1, 0, 0);
        }

#if !defined(CREATE_D2D_OVERLAY)
        // when d2d overlay is used, the transition occurs
        // when the resource is released by the overlay itself
        // // i.e. during the this->present_target() call
        // see also the comment in
        // ReleaseWrappedResource in trrojan::d3d12::d2d_overlay::end_draw()
        this->disable_target(cmd_list, i);
#endif // defined(CREATE_D2D_OVERLAY)
        close_command_list(cmd_list);
    }

#if 1
    // Do prewarming and compute number of CPU iterations at the same time.
    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto batch_time = 0.0;
        auto prewarms = (std::max)(1u, cfg.min_prewarms());

        do {
            mctx.cpu_timer.start();
            for (mctx.cpu_iterations = 0; mctx.cpu_iterations < prewarms;
                    ++mctx.cpu_iterations) {
                auto cmd_list = cmd_lists[this->buffer_index()];
                device.execute_command_list(cmd_list);
                this->present_target(config);
            }
            device.wait_for_gpu();

            prewarms = mctx.check_cpu_iterations(cfg.min_wall_time(),
                cfg.prewarm_precision());
        } while (prewarms > 0);
    }
#endif

#if 1
    // Do the wall clock measurement using the prepared command lists.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", mctx.cpu_iterations);
    mctx.cpu_timer.start();
    for (std::uint32_t i = 0; i < mctx.cpu_iterations; ++i) {
        auto cmd_list = cmd_lists[this->buffer_index()];
        device.execute_command_list(cmd_list);
        this->present_target(config);
    }
    device.wait_for_gpu();
    const auto cpu_time = mctx.cpu_timer.elapsed_millis();
#endif

#if 1
    // Do the GPU counter measurements using individual command lists.
    bundle_times.resize(cfg.gpu_counter_iterations());
    gpu_times.resize(cfg.gpu_counter_iterations());
    for (std::uint32_t i = 0; i < cfg.gpu_counter_iterations(); ++i) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", i);
        auto cmd_list = cmd_lists[this->buffer_index()];
        this->reset_command_list(cmd_list);

        cmd_list->SetGraphicsRootSignature(root_sig);
        set_descriptors(cmd_list, desc_tables);

        mctx.gpu_timer.start_frame();
        mctx.gpu_timer.start(cmd_list, 0);

        this->enable_target(cmd_list);
        this->clear_target(cmd_list);

        mctx.gpu_timer.start(cmd_list, 1);
        cmd_list->ExecuteBundle(bundle);
        mctx.gpu_timer.end(cmd_list, 1);

#if !defined(CREATE_D2D_OVERLAY)
        // when overlay active, target gets transitioned to PRESENT after overlay rendering
        // i.e. during this->present_target() call
        this->disable_target(cmd_list);
#endif // defined(CREATE_D2D_OVERLAY)
        mctx.gpu_timer.end(cmd_list, 0);
        const auto timer_index = mctx.gpu_timer.end_frame(cmd_list);

        device.close_and_execute_command_list(cmd_list);
        this->present_target(config);

        device.wait_for_gpu();
        gpu_times[i] = gpu_timer::to_milliseconds(
            mctx.gpu_timer.evaluate(timer_index, 0),
            gpu_freq);
        bundle_times[i] = gpu_timer::to_milliseconds(
            mctx.gpu_timer.evaluate(timer_index, 1),
            gpu_freq);
    }
#endif

    // Obtain pipeline statistics.
    log::instance().write_line(log_level::debug, "Collecting pipeline "
        "statistics ...");
    {
        auto cmd_list = cmd_lists[this->buffer_index()];
        this->reset_command_list(cmd_list);

        cmd_list->SetGraphicsRootSignature(root_sig);
        set_descriptors(cmd_list, desc_tables);

        stats_query.begin_frame();

        this->enable_target(cmd_list);
        this->clear_target(cmd_list);

        stats_query.begin(cmd_list, 0);
        cmd_list->ExecuteBundle(bundle);
        stats_query.end(cmd_list, 0);

#if !defined(CREATE_D2D_OVERLAY)
        // when overlay active, target gets transitioned to PRESENT after overlay rendering
        // i.e. during this->present_target() call
        this->disable_target(cmd_list);
#endif // defined(CREATE_D2D_OVERLAY)
        const auto stats_index = stats_query.end_frame(cmd_list);

        device.close_and_execute_command_list(cmd_list);
        this->present_target(config);

        // Wait until the results are here.
        device.wait_for_gpu();

        pipeline_stats = stats_query.evaluate(stats_index, 0);
    }

#if 1
    // Compute derived statistics for GPU counters.
    const auto bundle_median = calc_median(bundle_times);
    const auto gpu_median = calc_median(gpu_times);
#endif

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
        this->_data.spheres(),
        this->_data.extents(),
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
        mctx.cpu_iterations,
        cpu_time,
        static_cast<double>(cpu_time) / mctx.cpu_iterations
    });

    return retval;
}
