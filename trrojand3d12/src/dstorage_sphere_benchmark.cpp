﻿// <copyright file="dstorage_sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#if defined(TRROJAN_WITH_DSTORAGE)
#include "trrojan/d3d12/dstorage_sphere_benchmark.h"

#include "trrojan/text.h"

#include "trrojan/d3d12/measurement_context.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"


#define _DSTOR_BENCH_DEFINE_FACTOR(f)                                          \
const char *trrojan::d3d12::dstorage_sphere_benchmark::factor_##f = #f

_DSTOR_BENCH_DEFINE_FACTOR(queue_priority);
_DSTOR_BENCH_DEFINE_FACTOR(staging_directory);

#undef _DSTOR_BENCH_DEFINE_FACTOR


/*
 * ...::dstorage_sphere_benchmark::dstorage_sphere_benchmark
 */
trrojan::d3d12::dstorage_sphere_benchmark::dstorage_sphere_benchmark(
        void) : sphere_benchmark_base("dstorage-sphere-renderer"),
        _next_fence_value(0) {
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_count, 8u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_size, 1024u));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_queue_priority,
        static_cast<std::uint32_t>(DSTORAGE_PRIORITY_NORMAL)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_staging_directory, get_temp_folder()));
}


/*
 * trrojan::d3d12::dstorage_sphere_benchmark::check_stream_changed
 */
bool trrojan::d3d12::dstorage_sphere_benchmark::check_stream_changed(
        d3d12::device& device, const configuration& config,
        const std::vector<std::string>& changed) {
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
 * trrojan::d3d12::dstorage_sphere_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::dstorage_sphere_benchmark::count_descriptor_tables(
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
 * trrojan::d3d12::dstorage_sphere_benchmark::on_device_switch
 */
void trrojan::d3d12::dstorage_sphere_benchmark::on_device_switch(
        device& device) {
    assert(device.d3d_device() != nullptr);
    sphere_benchmark_base::on_device_switch(device);
}


/*
 * trrojan::d3d12::dstorage_sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::dstorage_sphere_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    std::vector<gpu_timer::millis_type> batch_times, gpu_times;
    sphere_rendering_configuration cfg(config);
    auto cmd_queue = device.command_queue();
    winrt::com_ptr<IDStorageFactory> dstorage;
    winrt::com_ptr<IDStorageFile> file;
    const auto folder = config.get<std::string>(factor_staging_directory);
    const auto gpu_freq = gpu_timer::get_timestamp_frequency(cmd_queue.p);
    winrt::com_ptr<IDStorageQueue> io_queue;
    measurement_context mctx(device, 2, this->pipeline_depth());
    stats_query::value_type pipeline_stats;
    DSTORAGE_REQUEST request;
    auto shader_code = cfg.shader_id();

    // Clear data that cannot be used any more.
    if (this->clear_stale_data(changed)) {
        ::DeleteFileW(this->_path.c_str());
    }

    // Load the data if necessary and copy it to the staging location requested
    // by the configuration. Note that we need to preserver the path of the
    // staged file, because we will not redo this until the data set changes
    // again.
    if (!this->_data) {
        const auto path = create_temp_file(folder, "tds");
        log::instance().write_line(log_level::information, "Staging data set \""
            "{0}\" into \"{1}\" ...", cfg.data_set(), path);
        this->_data.copy_to(path, shader_code, cfg).close();
        this->_path = from_utf8(path);

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
        list->Close();
        cmd_lists.push_back(list);
#if (defined(DEBUG) || defined(_DEBUG))
        log::instance().write_line(log_level::debug, "Command list {0:p} is "
            "used for batch {1}.", static_cast<void *>(list.get()), b);
#endif  /* (defined(DEBUG) || defined(_DEBUG)) */
    }

    // Update constant buffers. These will not change for this run.
    this->update_constants(cfg, 0);

    // Prepare DirectStorage and open the staged file with DirectStorage.
    {
        auto hr = ::DStorageGetFactory(IID_PPV_ARGS(dstorage.put()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = dstorage->OpenFile(this->_path.c_str(),
            IID_PPV_ARGS(file.put()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        const auto priority = static_cast<DSTORAGE_PRIORITY>(
            config.get<std::uint32_t>(factor_queue_priority));
        DSTORAGE_QUEUE_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
        desc.Priority = priority;
        desc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
        desc.Device = device.d3d_device();

        auto hr = dstorage->CreateQueue(&desc, IID_PPV_ARGS(io_queue.put()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Prepare the fence we use to signal the GPU that a batch is ready.
    {
        this->_next_fence_value = 0;
        auto hr = device.d3d_device()->CreateFence(this->_next_fence_value,
            D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(this->_fence.put()));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Initialise the parts of the DirectStorage request that never change,
    // because we always stream from the same file to the same buffer, only at
    // different locations.
    ::ZeroMemory(&request, sizeof(request));
    request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
    request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_BUFFER;
    request.Source.File.Source = file.get();
    request.Destination.Buffer.Resource = this->_stream.buffer().get();

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
                    auto& alloc = this->_direct_cmd_allocators[b];
                    auto& list = cmd_lists[b];
                    const auto first = (t == 0);
                    const auto last = (t == last_batch);

                    // List must have been completed at this point, so we can
                    // reset now. Afterwards, reset the command list that used 
                    // the allocator.
                    alloc->Reset();
                    list->Reset(alloc, pipeline);

                    // Set the render target, and if this is the first batch,
                    // transition it as well.
                    this->enable_target(list.get(),
                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                        first);

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

                    // Issue the I/O request and enqueue a signal in the stream
                    // that the GPU can wait for.
                    request.Source.File.Offset = this->_stream.offset(t);
                    request.Source.File.Size = this->_stream.batch_size(t);
                    request.Destination.Buffer.Offset = this->_stream.offset(b);
                    request.Destination.Buffer.Size = request.Source.File.Size;
                    io_queue->EnqueueRequest(&request);
                    const auto fence_value = ++this->_next_fence_value;
                    io_queue->EnqueueSignal(this->_fence.get(), fence_value);
                    io_queue->Submit();

                    list->SetGraphicsRootSignature(root_sig);
                    list->IASetPrimitiveTopology(topology);
                    this->set_descriptors(list.get(), desc_tables);
                    this->set_vertex_buffer(list.get(), shader_code, b);

                    const auto counts = get_draw_count(shader_code, spheres);
                    list->DrawInstanced(counts.first, counts.second, 0, 0);

                    if (last) {
                        this->disable_target(list.get());
                    }

                    // Make command queue wait for I/O before rendering.
                    cmd_queue->Wait(this->_fence.get(), fence_value);
                    device.close_and_execute_command_list(list.get());

                    // Schedule a signal after we have submitted the command
                    // list for batch 'b'.
                    this->_stream.signal_done(b, cmd_queue);
 
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
            auto& alloc = this->_direct_cmd_allocators[b];
            auto& list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            // List must have been completed at this point, so we can
            // reset now. Afterwards, reset the command list that used 
            // the allocator.
            alloc->Reset();
            list->Reset(alloc, pipeline);

            // Set the render target, and if this is the first batch,
            // transition it as well.
            this->enable_target(list.get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                first);

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

            // Issue the I/O request and enqueue a signal in the stream
            // that the GPU can wait for.
            request.Source.File.Offset = this->_stream.offset(t);
            request.Source.File.Size = this->_stream.batch_size(t);
            request.Destination.Buffer.Offset = this->_stream.offset(b);
            request.Destination.Buffer.Size = request.Source.File.Size;
            io_queue->EnqueueRequest(&request);
            const auto fence_value = ++this->_next_fence_value;
            io_queue->EnqueueSignal(this->_fence.get(), fence_value);
            io_queue->Submit();

            list->SetGraphicsRootSignature(root_sig);
            list->IASetPrimitiveTopology(topology);
            this->set_descriptors(list.get(), desc_tables);
            this->set_vertex_buffer(list.get(), shader_code, b);

            const auto counts = get_draw_count(shader_code, spheres);
            list->DrawInstanced(counts.first, counts.second, 0, 0);

            if (last) {
                this->disable_target(list.get());
            }

            // Make command queue wait for I/O before rendering.
            cmd_queue->Wait(this->_fence.get(), fence_value);
            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

            // If this was the last batch, we need to swap the buffer.
            if (last) {
                this->present_target(config);
            }
        } /* for (std::size_t t = 0; t < total_batches; ++t) */
    }
    device.wait_for_gpu();
#endif
    const auto cpu_time = mctx.cpu_timer.elapsed_millis();
    const auto cnt_stalls = this->_stream.reset_stalls();

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
            auto& alloc = this->_direct_cmd_allocators[b];
            auto& list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            // List must have been completed at this point, so we can
            // reset now. Afterwards, reset the command list that used 
            // the allocator.
            alloc->Reset();
            list->Reset(alloc, pipeline);

            // Set the render target, and if this is the first batch,
            // transition it as well.
            this->enable_target(list.get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                first);

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

            // Issue the I/O request and enqueue a signal in the stream
            // that the GPU can wait for.
            request.Source.File.Offset = this->_stream.offset(t);
            request.Source.File.Size = this->_stream.batch_size(t);
            request.Destination.Buffer.Offset = this->_stream.offset(b);
            request.Destination.Buffer.Size = request.Source.File.Size;
            io_queue->EnqueueRequest(&request);
            const auto fence_value = ++this->_next_fence_value;
            io_queue->EnqueueSignal(this->_fence.get(), fence_value);
            io_queue->Submit();

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

            if (last) {
                this->disable_target(list.get());
            }

            // Make command queue wait for I/O before rendering.
            cmd_queue->Wait(this->_fence.get(), fence_value);
            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

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
            auto& alloc = this->_direct_cmd_allocators[b];
            auto& list = cmd_lists[b];
            const auto first = (t == 0);
            const auto last = (t == last_batch);

            // List must have been completed at this point, so we can
            // reset now. Afterwards, reset the command list that used 
            // the allocator.
            alloc->Reset();
            list->Reset(alloc, pipeline);

            // Set the render target, and if this is the first batch,
            // transition it as well.
            this->enable_target(list.get(),
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                first);

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

            // Issue the I/O request and enqueue a signal in the stream
            // that the GPU can wait for.
            request.Source.File.Offset = this->_stream.offset(t);
            request.Source.File.Size = this->_stream.batch_size(t);
            request.Destination.Buffer.Offset = this->_stream.offset(b);
            request.Destination.Buffer.Size = request.Source.File.Size;
            io_queue->EnqueueRequest(&request);
            const auto fence_value = ++this->_next_fence_value;
            io_queue->EnqueueSignal(this->_fence.get(), fence_value);
            io_queue->Submit();

            list->SetGraphicsRootSignature(root_sig);
            list->IASetPrimitiveTopology(topology);
            this->set_descriptors(list.get(), desc_tables);
            this->set_vertex_buffer(list.get(), shader_code, b);

            const auto counts = get_draw_count(shader_code, spheres);
            stats_query.begin(list.get(), t);
            list->DrawInstanced(counts.first, counts.second, 0, 0);
            stats_query.end(list.get(), t);

            if (last) {
                this->disable_target(list.get());
                stats_index = stats_query.end_frame(list.get());
            }

            // Make command queue wait for I/O before rendering.
            cmd_queue->Wait(this->_fence.get(), fence_value);
            device.close_and_execute_command_list(list.get());

            // Schedule a signal after we have submitted the command
            // list for batch 'b'.
            this->_stream.signal_done(b, this->command_queue());

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
 * trrojan::d3d12::dstorage_sphere_benchmark::set_vertex_buffer
 */
void trrojan::d3d12::dstorage_sphere_benchmark::set_vertex_buffer(
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
#endif /* defined(TRROJAN_WITH_DSTORAGE) */