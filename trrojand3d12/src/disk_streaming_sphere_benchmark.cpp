﻿// <copyright file="disk_streaming_sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/disk_streaming_sphere_benchmark.h"

#include "trrojan/d3d12/measurement_context.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"


/*
 * ...::disk_streaming_sphere_benchmark::disk_streaming_sphere_benchmark
 */
trrojan::d3d12::disk_streaming_sphere_benchmark::disk_streaming_sphere_benchmark(
        void) : sphere_benchmark_base("disk-stream-sphere-renderer") {
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_count, 8u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_size, 1024u));
}


/*
 * trrojan::d3d12::disk_streaming_sphere_benchmark::check_stream_changed
 */
bool trrojan::d3d12::disk_streaming_sphere_benchmark::check_stream_changed(
        d3d12::device& device, const configuration& config,
        const std::vector<std::string> &changed) {
    const auto retval = this->_stream.rebuild_required(changed);

    if (retval) {
        log::instance().write_line(log_level::information, "(Re-) Building GPU "
            "stream on device 0x{0:p} ...",
            static_cast<void *>(device.d3d_device().p));
        this->_stream.rebuild(device.d3d_device(), config);

        // We need an allocator for each batch we are rendering. In contrast to
        // the base class, we do not create allocators on a per-frame basis, but
        // on a per-batch basis. This is possible, because the pipeline does not
        // change between frames in the streaming benchmarks. We just swap what
        // is in the persistenly mapped data buffer, and we do not care whether
        // we swap this content because of moving to the next batch or because
        // of moving to the next frame. The only command lists that require
        // special handling are the one at the begin of the frame and the one
        // at the end of the frame, because these need to transition the render
        // target from present to rendering state and vice versa.
        this->_direct_cmd_allocators.clear();
        create_command_allocators(this->_direct_cmd_allocators,
            device.d3d_device(),
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            this->_stream.batch_count() + 2);
    }

    return retval;
}


/*
 * trrojan::d3d12::disk_streaming_sphere_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::disk_streaming_sphere_benchmark::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Let the base class compute how many descriptors we need for emitting one
    // draw call.
    auto retval = sphere_benchmark_base::count_descriptor_tables(shader_code,
        include_root);

    // All of the above is required for each batch that can run in parallel.
    retval *= static_cast<UINT>(this->_stream.batch_count());

    return retval;
}


/*
 * trrojan::d3d12::disk_streaming_sphere_benchmark::on_device_switch
 */
void trrojan::d3d12::disk_streaming_sphere_benchmark::on_device_switch(
        device& device) {
    assert(device.d3d_device() != nullptr);
    sphere_benchmark_base::on_device_switch(device);
}


/*
 * trrojan::d3d12::disk_streaming_sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::disk_streaming_sphere_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    std::vector<gpu_timer::millis_type> batch_times, gpu_times;
    sphere_rendering_configuration cfg(config);
    const auto file_path = create_temp_file("tdsb");
    const auto gpu_freq = gpu_timer::get_timestamp_frequency(
        device.command_queue());
    measurement_context mctx(device, 2, this->pipeline_depth());
    stats_query::value_type pipeline_stats;
    auto shader_code = cfg.shader_id();

    // Clear data that cannot be used any more.
    this->clear_stale_data(changed);

    // Load the data if necessary. Note that we redirect the data loaded into
    // '_buffer' rather than letting '_data' upload it directly to the GPU.
    if (!this->_data) {
        log::instance().write_line(log_level::information, "Copying raw data \""
            "{0}\" to \"{1}\" ...", cfg.data_set(), file_path);
        this->_data.copy_to(file_path, shader_code, cfg);
        log::instance().write_line(log_level::debug, "Reshaping GPU "
            "stream ...");
        this->_stream.reshape(this->_data);
    }


    // Now that we have the data, update the shader code to match it.
    shader_code = this->_data.adjust_shader_code(shader_code);

    // As we have the data, we can also configure the camera now.
    this->configure_camera(config);

    // If necessary, (re-) build the GPU heap and command allocators for
    // streaming the data.
    this->check_stream_changed(device, config, changed);

    // Now that we have reshaped the stream, we can cache the total number
    // of batches we need to render for each frame.
    const auto total_batches = this->_stream.total_batches();
    const auto last_batch = total_batches - 1;

    // Get the number of descriptors we need for each batch. This information is
    // provided by the base class. Note that our implementation is overrridden
    // to yxield the total number of descriptors for all batches.
    const auto cnt_descs = sphere_benchmark_base::count_descriptor_tables(
        shader_code, true);

    // Prepare the descriptor heaps required for the requested technique and
    // data set. Note that create_descriptor_heap will access the data
    // properties, so this must be done *after* loading the data. Also note
    // that, as we override count_descriptor_tables, the base class will
    // allocate enought slots for all the batches we will render.
    this->create_descriptor_heaps(device.d3d_device(), shader_code);

    auto pipeline = this->get_pipeline_state(device.d3d_device(), shader_code);
    auto root_sig = this->get_root_signature(device.d3d_device(), shader_code);
    auto topology = get_primitive_topology(shader_code);
    auto buffer = this->_stream.buffer().get();

    // Prepare a command list for each batch, which we will repeatedly call until
    // all of the data have been streamed to the GPU. The index of the list
    // specifies the index of the index of the batch, ie if we filled the buffer
    // for the first batch, we need to render the first command list afterwards.
    std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> cmd_lists;
    cmd_lists.reserve(this->_stream.batch_count());
    for (std::size_t b = 0; (b < this->_stream.batch_count()); ++b) {
        auto list = to_winrt(this->create_graphics_command_list(b, pipeline));
        set_debug_object_name(list.get(), "Command list for batch {0}", b);
        cmd_lists.push_back(list);
    }

    // Update constant buffers. These will not change for this run.
    this->update_constants(cfg, 0);

#if true
    // Prewarm rendering such that shaders are ready. This also computes the
    // number of iterations for the wall clock measurements.
    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto prewarms = (std::max)(1u, cfg.min_prewarms());

        do {
            mctx.cpu_timer.start();
            for (mctx.cpu_iterations = 0; mctx.cpu_iterations < prewarms;
                    ++mctx.cpu_iterations) {
                const auto f = this->buffer_index();

                for (std::size_t t = 0; t < total_batches; ++t) {
                    // Within this loop, 't' is the global index of the batch,
                    // which is used to address the source data, whereas 'b' is
                    // the position in the ring buffer on the GPU.
                    const auto b = this->_stream.next_batch();
                    //log::instance().write_line(log_level::debug,
                    //    "Using batch {0} for {1} of {2}...", b, t,
                    //    total_batches);
                    auto list = cmd_lists[b];
                    const auto first = (t == 0);
                    const auto last = (t == last_batch);

                    this->enable_target(list.get());

                    // If this is the first batch, we need to clear the target.
                    if (first) {
                        this->clear_target(list.get());
                    }

                    const auto spheres = this->_stream.batch_elements(t);
                    auto desc_tables = this->set_descriptors(
                        device.d3d_device(),
                        shader_code,
                        0,
                        b * cnt_descs,
                        buffer,
                        b * this->_stream.batch_elements(),
                        spheres);

                    //::memcpy(this->_stream.data(b),
                    //    this->_buffer.data() + this->_stream.offset(t),
                    //    this->_stream.batch_size(t));

                    list->SetGraphicsRootSignature(root_sig);
                    list->IASetPrimitiveTopology(topology);
                    this->set_descriptors(list.get(), desc_tables);
                    this->set_vertex_buffer(list.get(), shader_code, b);

                    const auto counts = get_draw_count(shader_code, spheres);
                    list->DrawInstanced(counts.first, counts.second, 0, 0);

                    device.close_and_execute_command_list(list.get());

                    // Schedule a signal after we have submitted the command
                    // list for batch 'b'.
                    this->_stream.signal_done(b, this->command_queue());

                    // Immediately reset the list for the next loop. It is OK to
                    // reset a command list once it has been submitted, which is
                    // in contrast to the underlying allocator.
                    list->Reset(this->_direct_cmd_allocators[b], pipeline);
 
                    // If this was the last batch, we need to swap the buffer.
                    if (last) {
                        this->present_target(config);
                    }
                } /* for (std::size_t t = 0; t < total_batches; ++t) */
            }
            device.wait_for_gpu();
            prewarms = mctx.check_cpu_iterations(cfg.min_wall_time());
        } while (prewarms > 0);
    }
#endif

#if true
    // Do the wall clock measurements.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", mctx.cpu_iterations);
    this->_stream.reset_stalls();
    mctx.cpu_timer.start();
    for (std::uint32_t i = 0; i < mctx.cpu_iterations; ++i) {
        for (std::size_t t = 0; t < total_batches; ++t) {
            // Within this loop, 't' is the global index of the batch,
            // which is used to address the source data, whereas 'b' is
            // the position in the ring buffer on the GPU.
            const auto b = this->_stream.next_batch();
            //log::instance().write_line(log_level::debug,
            //    "Using batch {0} for {1} of {2}...", b, t,
            //    total_batches);
            auto list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            this->enable_target(list.get());

            // If this is the first batch, we need to clear the target.
            if (first) {
                this->clear_target(list.get());
            }

            const auto spheres = this->_stream.batch_elements(t);
            auto desc_tables = this->set_descriptors(
                device.d3d_device(),
                shader_code,
                0,
                b * cnt_descs,
                buffer,
                b * this->_stream.batch_elements(),
                spheres);

            //::memcpy(this->_stream.data(b),
            //    this->_buffer.data() + this->_stream.offset(t),
            //    this->_stream.batch_size(t));

            list->SetGraphicsRootSignature(root_sig);
            list->IASetPrimitiveTopology(topology);
            this->set_descriptors(list.get(), desc_tables);
            this->set_vertex_buffer(list.get(), shader_code, b);

            const auto counts = get_draw_count(shader_code, spheres);
            list->DrawInstanced(counts.first, counts.second, 0, 0);

            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

            // Immediately reset the list for the next loop. It is OK to
            // reset a command list once it has been submitted, which is
            // in contrast to the underlying allocator.
            list->Reset(this->_direct_cmd_allocators[b], pipeline);

            // If this was the last batch, we need to swap the buffer.
            if (last) {
                this->present_target(config);
            }
        } /* for (std::size_t t = 0; t < total_batches; ++t) */
    }
    device.wait_for_gpu();
    const auto cpu_time = mctx.cpu_timer.elapsed_millis();
    const auto cnt_stalls = this->_stream.reset_stalls();
#endif

#if true
    // Do the GPU counter measurements.
    batch_times.reserve(total_batches * cfg.gpu_counter_iterations());
    gpu_times.resize(cfg.gpu_counter_iterations());
    for (std::uint32_t i = 0; i < cfg.gpu_counter_iterations(); ++i) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", i);

        gpu_timer::size_type timer_index = 0;
        mctx.gpu_timer.start_frame();

        for (std::size_t t = 0; t < total_batches; ++t) {
            // Within this loop, 't' is the global index of the batch,
            // which is used to address the source data, whereas 'b' is
            // the position in the ring buffer on the GPU.
            const auto b = this->_stream.next_batch();
            //log::instance().write_line(log_level::debug,
            //    "Using batch {0} for {1} of {2}...", b, t,
            //    total_batches);
            auto list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            this->enable_target(list.get());

            // If this is the first batch, we need to clear the target.
            if (first) {
                mctx.gpu_timer.start(list.get(), 0);
                this->clear_target(list.get());
            }

            mctx.gpu_timer.start(list.get(), 1);

            const auto spheres = this->_stream.batch_elements(t);
            auto desc_tables = this->set_descriptors(
                device.d3d_device(),
                shader_code,
                0,
                b * cnt_descs,
                buffer,
                b * this->_stream.batch_elements(),
                spheres);

            //::memcpy(this->_stream.data(b),
            //    this->_buffer.data() + this->_stream.offset(t),
            //    this->_stream.batch_size(t));

            list->SetGraphicsRootSignature(root_sig);
            list->IASetPrimitiveTopology(topology);
            this->set_descriptors(list.get(), desc_tables);
            this->set_vertex_buffer(list.get(), shader_code, b);

            const auto counts = get_draw_count(shader_code, spheres);
            list->DrawInstanced(counts.first, counts.second, 0, 0);

            mctx.gpu_timer.end(list.get(), 1);
            if (last) {
                mctx.gpu_timer.end(list.get(), 0);
                timer_index = mctx.gpu_timer.end_frame(list.get());
            }

            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

            // Immediately reset the list for the next loop. It is OK to
            // reset a command list once it has been submitted, which is
            // in contrast to the underlying allocator.
            list->Reset(this->_direct_cmd_allocators[b], pipeline);

            // If this was the last batch, we need to swap the buffer.
            if (last) {
                this->present_target(config);
            }
        } /* for (std::size_t t = 0; t < total_batches; ++t) */

        device.wait_for_gpu();
        gpu_times[i] = gpu_timer::to_milliseconds(
            mctx.gpu_timer.evaluate(timer_index, 0),
            gpu_freq);
        batch_times.push_back(gpu_timer::to_milliseconds(
            mctx.gpu_timer.evaluate(timer_index, 1),
            gpu_freq));
    } /* for (std::uint32_t i = 0; i < cfg.gpu_counter_iterations(); ++i) */
#endif

#if true
    // Obtain pipeline statistics.
    log::instance().write_line(log_level::debug, "Collecting pipeline "
        "statistics ...");
    {
        // Allocate queries for each batch, because we cannot span the queries
        // over multiple command lists.
        stats_query stats_query(device.d3d_device(),
            this->_stream.total_batches(), 1);

        stats_query::size_type stats_index = 0;
        stats_query.begin_frame();

        for (std::size_t t = 0; t < total_batches; ++t) {
            // Within this loop, 't' is the global index of the batch,
            // which is used to address the source data, whereas 'b' is
            // the position in the ring buffer on the GPU.
            const auto b = this->_stream.next_batch();
            //log::instance().write_line(log_level::debug,
            //    "Using batch {0} for {1} of {2}...", b, t,
            //    total_batches);
            auto list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            this->enable_target(list.get());

            // If this is the first batch, we need to clear the target.
            if (first) {
                this->clear_target(list.get());
            }

            const auto spheres = this->_stream.batch_elements(t);
            auto desc_tables = this->set_descriptors(
                device.d3d_device(),
                shader_code,
                0,
                b * cnt_descs,
                buffer,
                b * this->_stream.batch_elements(),
                spheres);

            //::memcpy(this->_stream.data(b),
            //    this->_buffer.data() + this->_stream.offset(t),
            //    this->_stream.batch_size(t));

            list->SetGraphicsRootSignature(root_sig);
            list->IASetPrimitiveTopology(topology);
            this->set_descriptors(list.get(), desc_tables);
            this->set_vertex_buffer(list.get(), shader_code, b);

            const auto counts = get_draw_count(shader_code, spheres);
            stats_query.begin(list.get(), t);
            list->DrawInstanced(counts.first, counts.second, 0, 0);
            stats_query.end(list.get(), t);

            if (last) {
                stats_index = stats_query.end_frame(list.get());
            }

            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

            // Immediately reset the list for the next loop. It is OK to
            // reset a command list once it has been submitted, which is
            // in contrast to the underlying allocator.
            list->Reset(this->_direct_cmd_allocators[b], pipeline);

            // If this was the last batch, we need to swap the buffer.
            if (last) {
                this->present_target(config);
            }
        } /* for (std::size_t t = 0; t < total_batches; ++t) */

        // Wait until the results are here.
        device.wait_for_gpu();

        // Accummulate all stats of all batches.
        ::ZeroMemory(&pipeline_stats, sizeof(pipeline_stats));
        for (std::size_t i = 0; i < total_batches; ++i) {
            auto s = stats_query.evaluate(stats_index, i);
            pipeline_stats.IAVertices += s.IAVertices;
            pipeline_stats.IAPrimitives += s.IAPrimitives;
            pipeline_stats.VSInvocations += s.VSInvocations;
            pipeline_stats.GSInvocations += s.GSInvocations;
            pipeline_stats.GSPrimitives += s.GSPrimitives;
            pipeline_stats.CInvocations += s.CInvocations;
            pipeline_stats.CPrimitives += s.CPrimitives;
            pipeline_stats.PSInvocations += s.PSInvocations;
            pipeline_stats.HSInvocations += s.HSInvocations;
            pipeline_stats.DSInvocations += s.DSInvocations;
            pipeline_stats.CSInvocations += s.CSInvocations;
        }
    }
#endif

    // Compute derived statistics for GPU counters.
    const auto batch_median = calc_median(batch_times);
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
            "upload_stalls",
            "batch_time_min",
            "batch_time_med",
            "batch_time_max",
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
        cnt_stalls,
        batch_times.front(),
        batch_median,
        batch_times.back(),
        gpu_times.front(),
        gpu_median,
        gpu_times.back(),
        mctx.cpu_iterations,
        cpu_time,
        static_cast<double>(cpu_time) / mctx.cpu_iterations
    });

    return retval;
}


/*
 * trrojan::d3d12::disk_streaming_sphere_benchmark::set_vertex_buffer
 */
void trrojan::d3d12::disk_streaming_sphere_benchmark::set_vertex_buffer(
        ID3D12GraphicsCommandList *cmd_list,
        const shader_id_type shader_code,
        const std::size_t batch) {
    assert(cmd_list != nullptr);
    if (!is_technique(shader_code, SPHERE_TECHNIQUE_USE_SRV)) {
        D3D12_VERTEX_BUFFER_VIEW desc;
        desc.BufferLocation = this->_stream.descriptor(batch);
        desc.SizeInBytes = this->_stream.batch_size(0);
        desc.StrideInBytes = this->_data.stride();

#if (defined(DEBUG) || defined(_DEBUG))
        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "vertex buffer. Setting 0x{0:x} ...", desc.BufferLocation);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        cmd_list->IASetVertexBuffers(0, 1, &desc);
    }
}