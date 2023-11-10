// <copyright file="ram_streaming_sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/ram_streaming_sphere_benchmark.h"

#include "trrojan/d3d12/measurement_context.h"

#include "sphere_techniques.h"


/*
 * ...::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark
 */
trrojan::d3d12::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark(
        void) : sphere_benchmark_base("ram-stream-sphere-renderer") {
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_count, 8u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_size, 1024u));
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::check_stream_changed
 */
void trrojan::d3d12::ram_streaming_sphere_benchmark::check_stream_changed(
        d3d12::device& device, const configuration& config,
        const std::vector<std::string> &changed) {
    if (this->_stream.rebuild_required(changed)) {
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
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Let the base class compute how many descriptors we need for emitting one
    // draw call.
    auto retval = sphere_benchmark_base::count_descriptor_tables(shader_code,
        false);

    // All of the above is required for each batch that can run in parallel.
    retval *= static_cast<UINT>(this->_stream.batch_count());

    if (include_root) {
        ++retval;
    }

    return retval;
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::on_device_switch
 */
void trrojan::d3d12::ram_streaming_sphere_benchmark::on_device_switch(
        device& device) {
    assert(device.d3d_device() != nullptr);
    sphere_benchmark_base::on_device_switch(device);
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d12::ram_streaming_sphere_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    sphere_rendering_configuration cfg(config);
    measurement_context mctx(device, 1, this->pipeline_depth()); // TODO: needs to know # of batches
    auto shader_code = cfg.shader_id();

    // Clear data that cannot be used any more.
    this->clear_stale_data(changed);

    // Load the data if necessary.
    if (!this->_data) {
        log::instance().write_line(log_level::information, "Loading data set \""
            "{}\" into memory ...", cfg.data_set());
        this->_data.load([this](const UINT64 size) {
            this->_buffer.resize(size);
            return this->_buffer.data();
        }, shader_code, cfg);

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
    // of batches we need to render for each frame and the index of the
    // last one, which is potentially smaller and needs to transition the
    // render target into present state. 'begin_index' and 'end_index' are
    // the position of the command lists that start or end a frame.
    const auto total_batches = this->_stream.total_batches();
    const auto last_batch = total_batches - 1;
    const auto begin_index = this->_stream.batch_count();
    const auto end_index = begin_index + 1;

    // Get the number of descriptors we need for each batch.
    const auto cnt_descs = sphere_benchmark_base::count_descriptor_tables(
        shader_code, false);

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

#if 0
    // Record a command list for each batch, which we will repeatedly call until
    // all of the data have been streamed to the GPU. The index of the list
    // specifies the index of the index of the batch, ie if we filled the buffer
    // for the first batch, we need to render the first command list afterwards.
    std::vector<winrt::com_ptr<ID3D12GraphicsCommandList>> cmd_lists(
        this->_stream.batch_count());
    for (std::size_t b = 0; (b < this->_stream.batch_count())
            && (b < this->_stream.total_batches()); ++b) {
        auto list = cmd_lists[b] = to_winrt(
            this->create_graphics_command_list(b, pipeline));
        set_debug_object_name(list.get(), "Command list for batch {0}", b);

        const auto spheres = this->_stream.batch_elements();
        auto desc_tables = this->set_descriptors(device.d3d_device(),
            shader_code, 0, b * cnt_descs, buffer, this->_stream.offset(b),
            spheres);

        list->SetGraphicsRootSignature(root_sig);
        list->IASetPrimitiveTopology(topology);
        this->set_descriptors(list.get(), desc_tables);
        this->set_vertex_buffer(list.get(), shader_code, b);

        const auto counts = get_draw_count(shader_code, spheres);
        list->DrawInstanced(counts.first, counts.second, 0, 0);
        close_command_list(list.get());
    }
#endif

    // Prepare the command lists for the first and last batch of each frame.
    // These need to be filled on the fly once we know which of the buffers
    // we need to bind.
    auto begin_list = to_winrt(this->create_graphics_command_list(begin_index,
        pipeline));
    set_debug_object_name(begin_list.get(), "Begin frame command list");
    close_command_list(begin_list.get());
    auto end_list = to_winrt(this->create_graphics_command_list(end_index,
        pipeline));
    set_debug_object_name(end_list.get(), "End frame command list");
    close_command_list(end_list.get());

    // Update constant buffers.
    this->update_constants(cfg, 0);

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
                    auto b = this->_stream.next_batch();
                    log::instance().write_line(log_level::debug,
                        "Using batch {0} ...", b);

                    if (false&& (t > 0) && (t < last_batch)) {
                        // This is a "normal" batch, which we can just submit
                        // using the command lists we prepared before.
#if 0
                        auto list = cmd_lists[b];
                        ::memcpy(this->_stream.data(b),
                            this->_buffer.data() + this->_stream.offset(t),
                            this->_stream.batch_size(t));
                        device.execute_command_list(list.get());
#endif

                    } else {
                        // This is either the first batch, which needs to
                        // transition and clear the render target, or it is the
                        // last one, which needs to present the render target
                        // and as potentially less data in it. As the command
                        // list may be different every time, we need to reset
                        // and re-record it using the appropriate batch 'b'.
                        const auto first = (t == 0);
                        const auto last = (t == last_batch);
                        auto alloc = first
                            ? this->_direct_cmd_allocators[begin_index]
                            : this->_direct_cmd_allocators[end_index];
                        auto list = first ? begin_list : end_list;
                        list->Reset(alloc, pipeline);

                        if (first) {
                            this->enable_target(list.get());
                            this->clear_target(list.get());
                        }

                        const auto spheres = this->_stream.batch_elements(t);
                        auto desc_tables = this->set_descriptors(
                            device.d3d_device(), shader_code, 0,
                            b + cnt_descs, buffer,
                            b * this->_stream.batch_elements(), spheres);

                        ::memcpy(this->_stream.data(b),
                            this->_buffer.data() + this->_stream.offset(t),
                            this->_stream.batch_size(t));

                        list->SetGraphicsRootSignature(root_sig);
                        list->IASetPrimitiveTopology(topology);
                        this->set_descriptors(list.get(), desc_tables);
                        this->set_vertex_buffer(list.get(), shader_code, b);

                        const auto counts = get_draw_count(shader_code, spheres);
                        list->DrawInstanced(counts.first, counts.second, 0, 0);

                        if (last) {
                            this->present_target();
                        }

                        device.close_and_execute_command_list(list.get());
                    }
 
                    // Schedule a signal after we have submitted the command
                    // list for batch 'b'.
                    this->_stream.signal_done(b, this->command_queue());
                }
            }
            device.wait_for_gpu();
            prewarms = mctx.check_cpu_iterations(cfg.min_wall_time());
        } while (prewarms > 0);
    }


    return trrojan::result();
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::set_vertex_buffer
 */
void trrojan::d3d12::ram_streaming_sphere_benchmark::set_vertex_buffer(
        ID3D12GraphicsCommandList *cmd_list,
        const shader_id_type shader_code,
        const std::size_t batch) {
    assert(cmd_list != nullptr);
    if (!is_technique(shader_code, SPHERE_TECHNIQUE_USE_SRV)) {
        D3D12_VERTEX_BUFFER_VIEW desc;
        desc.BufferLocation = this->_stream.descriptor(batch);
        desc.SizeInBytes = this->_stream.batch_size(0);
        desc.StrideInBytes = this->_data.stride();

        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "vertex buffer. Setting 0x{0:x} ...", desc.BufferLocation);
        cmd_list->IASetVertexBuffers(0, 1, &desc);
    }
}
