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

    // (Re-) Create GPU queries.
    //this->stats_query = create_pipline_stats_query(dev);
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
    auto bundle = this->create_command_bundle(0, pipeline);

    //bundle->SetGraphicsRootSignature();
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
        //cmd_lists[i]->ExecuteBundle(bundle);
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
        //cmd_lists[i]->ExecuteBundle(bundle);
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




#if 0

/*
 * trrojan::d3d12::sphere_benchmark_base::on_run
 */
trrojan::result trrojan::d3d12::sphere_benchmark_base::on_run(d3d12::device &device,
    const configuration &config, const std::vector<std::string> &changed) {
    typedef rendering_technique::shader_stage shader_stage;

    std::array<float, 3> bboxSize;
    trrojan::timer cpuTimer;
    auto cntCpuIterations = static_cast<std::uint32_t>(0);
    const auto cntGpuIterations = config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    std::vector<gpu_timer_type::millis_type> gpuTimes;
    auto isDisjoint = true;
    const auto minWallTime = config.get<std::uint32_t>(factor_min_wall_time);
    d3d12_QUERY_DATA_PIPELINE_STATISTICS pipeStats;
    auto shaderCode = sphere_benchmark_base::get_shader_id(config);
    SphereConstants sphereConstants;
    TessellationConstants tessConstants;
    ViewConstants viewConstants;
    d3d12_VIEWPORT viewport;

    // If the device has changed, invalidate all GPU resources and recreate the
    // data-independent ones.
    if (contains(changed, factor_device)) {
        log::instance().write_line(log_level::verbose, "Preparing GPU "
            "resources for device \"{}\" ...", device.name().c_str());
        this->technique_cache.clear();

        if (this->data != nullptr) {
            this->data->release();
        }

        // Constant buffers.
        this->sphere_constants = create_buffer(dev, d3d12_USAGE_DEFAULT,
            d3d12_BIND_CONSTANT_BUFFER, nullptr, sizeof(SphereConstants));
        set_debug_object_name(this->sphere_constants.p, "sphere_constants");
        this->tessellation_constants = create_buffer(dev, d3d12_USAGE_DEFAULT,
            d3d12_BIND_CONSTANT_BUFFER, nullptr, sizeof(TessellationConstants));
        set_debug_object_name(this->tessellation_constants.p,
            "tessellation_constants");
        this->view_constants = create_buffer(dev, d3d12_USAGE_DEFAULT,
            d3d12_BIND_CONSTANT_BUFFER, nullptr, sizeof(ViewConstants));
        set_debug_object_name(this->view_constants.p, "view_constants");

        // Textures and SRVs.
        this->colour_map = nullptr;
        create_viridis_colour_map(dev, &this->colour_map);

        // Samplers.
        this->linear_sampler = create_linear_sampler(dev);

        // Queries.
        this->done_query = create_event_query(dev);
        this->stats_query = create_pipline_stats_query(dev);
    }

    // Determine whether the data set header must be loaded. This needs to be
    // done before any frame is loaded or the technique is selected.
    if (contains_any(changed, factor_data_set)) {
        this->data.reset();

        try {
            // Try to interpret the path as description of random spheres.
            this->make_random_spheres(dev, shaderCode, config);
            assert(this->check_data_compatibility(shaderCode));
        } catch (std::exception &ex) {
            // If parsing the path as random spheres failed, interpret it as
            // path to an MMPLD file.
            log::instance().write_line(log_level::warning, ex);

            auto path = config.get<std::string>(factor_data_set);
            log::instance().write_line(log_level::verbose, "Loading MMPLD data "
                "set \"{}\" ...", path.c_str());
            this->data = mmpld_data_set::create(path);
        }
    }
    /* At this point, the data header for processing the MMPLD is OK. */

    // For MMPLD data, we need to consider that there are existing frame data
    // which might be from the wrong frame or in the woring format. Therefore,
    // check frame and data compatibility and re-read the frame as necessary.
    // For random sphere data, we only need to consider that the existing data
    // are not compatible with the rendering technique.
    {
        auto m = std::dynamic_pointer_cast<mmpld_data_set>(this->data);
        if (m != nullptr) {
            auto isFrameChanged = contains_any(changed, factor_frame,
                factor_fit_bounding_box);
            auto isFrameCompat = this->check_data_compatibility(shaderCode);

            if (isFrameChanged || !isFrameCompat) {
                this->load_mmpld_frame(dev, shaderCode, config);
                assert(this->check_data_compatibility(shaderCode));
            }
        }

        auto r = std::dynamic_pointer_cast<random_sphere_data_set>(this->data);
        if (r != nullptr) {
            auto isFrameCompat = this->check_data_compatibility(shaderCode);

            if (!isFrameCompat) {
                log::instance().write_line(log_level::debug, "Recreating "
                    "random sphere data set due to incompatibility with "
                    "current rendering technique.");
                r->recreate(dev, shaderCode);
                assert(this->check_data_compatibility(shaderCode));
            }
        }
    }
    /* At this point, we should have valid data for the selected technique. */
    assert(this->check_data_compatibility(shaderCode));

    // The 'shaderCode' might have some flags preventively set to request
    // certain data properties. Remove the flags that cannot fulfilled by the
    // data
    if ((this->get_data_properties(shaderCode) & SPHERE_INPUT_PV_COLOUR) == 0) {
        shaderCode &= ~SPHERE_INPUT_FLT_COLOUR;
    }
    {
        static const auto INTENSITY_MASK = (SPHERE_INPUT_PV_INTENSITY
            | SPHERE_INPUT_PP_INTENSITY);
        if ((this->get_data_properties(shaderCode) & INTENSITY_MASK) == 0) {
            shaderCode &= ~INTENSITY_MASK;
        }
    }

    // Select or create the right rendering technique and apply the data set
    // to the technique.
    auto &technique = this->get_technique(dev, shaderCode);
    this->data->apply(technique, rendering_technique::combine_shader_stages(
        shader_stage::vertex), 0, 1);

    // Retrieve the viewport for rasteriser and shaders.
    {
        auto vp = config.get<viewport_type>(factor_viewport);
        viewport.TopLeftX = viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(vp[0]);
        viewport.Height = static_cast<float>(vp[1]);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        viewConstants.Viewport.x = 0.0f;
        viewConstants.Viewport.y = 0.0f;
        viewConstants.Viewport.z = viewport.Width;
        viewConstants.Viewport.w = viewport.Height;
    }

    // Initialise the GPU timer.
    gpuTimer.initialise(dev);

    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(std::move(config),
        std::initializer_list<std::string> {
        "particles",
            "data_extents",
            "ia_vertices",
            "ia_primitives",
            "vs_invokes",
            "hs_invokes",
            "ds_invokes",
            "gs_invokes",
            "gs_primitives",
            "ps_invokes",
            "gpu_time_min",
            "gpu_time_med",
            "gpu_time_max",
            "wall_time_iterations",
            "wall_time",
            "wall_time_avg"
    });


    // Update constant buffers.
    ctx->UpdateSubresource(this->sphere_constants.p, 0, nullptr,
        &sphereConstants, 0, 0);
    ctx->UpdateSubresource(this->tessellation_constants.p, 0, nullptr,
        &tessConstants, 0, 0);
    ctx->UpdateSubresource(this->view_constants.p, 0, nullptr,
        &viewConstants, 0, 0);

    // Configure the rasteriser.
    ctx->RSSetViewports(1, &viewport);
    //context->RSSetState(this->rasteriserState.Get());

    // Enable the technique.
    technique.apply(ctx);

    // Determine the number of primitives to emit.
    auto cntPrimitives = this->data->size();
    auto cntInstances = 0;
    auto isInstanced = false;
    if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
        // Instancing of quads requires 4 vertices per particle.
        cntInstances = cntPrimitives;
        cntPrimitives = 4;
        isInstanced = true;
    }

#if 0
    {
        auto soBuffer = create_buffer(dev, d3d12_USAGE_DEFAULT,
            d3d12_BIND_STREAM_OUTPUT, nullptr, 4000 * sizeof(float) * 4, 0);
        UINT soOffset[] = { 0 };
        ctx->SOSetTargets(1, &soBuffer.p, soOffset);
    }
#endif

    // Do prewarming and compute number of CPU iterations at the same time.
    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto batchTime = 0.0;
        auto cntPrewarms = (std::max)(1u,
            config.get<std::uint32_t>(factor_min_prewarms));

        do {
            cntCpuIterations = 0;
            assert(cntPrewarms >= 1);

            cpuTimer.start();
            for (; cntCpuIterations < cntPrewarms; ++cntCpuIterations) {
                this->clear_target();
                if (isInstanced) {
                    ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
                } else {
                    ctx->Draw(cntPrimitives, 0);
                }
                this->present_target();
#if defined(CREATE_D2D_OVERLAY)
                // Using the overlay will change the state such that we need to
                // re-apply it after presenting.
                technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
            }

            ctx->End(this->done_query);
            wait_for_event_query(ctx, this->done_query);
            batchTime = cpuTimer.elapsed_millis();

            if (batchTime < minWallTime) {
                cntPrewarms = static_cast<std::uint32_t>(std::ceil(
                    static_cast<double>(minWallTime * cntCpuIterations)
                    / batchTime));
                if (cntPrewarms < 1) {
                    cntPrewarms = 1;
                }
            }
        } while (batchTime < minWallTime);
    }

    // Do the GPU counter measurements
    gpuTimes.resize(cntGpuIterations);
    for (std::uint32_t i = 0; i < cntGpuIterations;) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", i);
        gpuTimer.start_frame();
        gpuTimer.start(0);
        this->clear_target();
        if (isInstanced) {
            ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
        } else {
            ctx->Draw(cntPrimitives, 0);
        }
#if defined(CREATE_D2D_OVERLAY)
        // Using the overlay will change the state such that we need to
        // re-apply it after presenting.
        technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
        gpuTimer.end(0);
        gpuTimer.end_frame();

        gpuTimer.evaluate_frame(isDisjoint, gpuFreq);
        if (!isDisjoint) {
            gpuTimes[i] = gpu_timer_type::to_milliseconds(
                gpuTimer.evaluate(0), gpuFreq);
            ++i;    // Only proceed in case of success.
        }
    }

    // Obtain pipeline statistics
    log::instance().write_line(log_level::debug, "Collecting pipeline "
        "statistics ...");
    this->clear_target();
    ctx->Begin(this->stats_query);
    if (isInstanced) {
        ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
    } else {
        ctx->Draw(cntPrimitives, 0);
    }
    ctx->End(this->stats_query);
    this->present_target();
#if defined(CREATE_D2D_OVERLAY)
    // Using the overlay will change the state such that we need to
    // re-apply it after presenting.
    technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
    wait_for_stats_query(pipeStats, ctx, this->stats_query);

    // Do the wall clock measurement.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", cntCpuIterations);
    cpuTimer.start();
    for (std::uint32_t i = 0; i < cntCpuIterations; ++i) {
        this->clear_target();
        if (isInstanced) {
            ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
        } else {
            ctx->Draw(cntPrimitives, 0);
        }
        this->present_target();
#if defined(CREATE_D2D_OVERLAY)
        // Using the overlay will change the state such that we need to
        // re-apply it after presenting.
        technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
    }
    ctx->End(this->done_query);
    wait_for_event_query(ctx, this->done_query);
    auto cpuTime = cpuTimer.elapsed_millis();

    // Compute derived statistics for GPU counters.
    std::sort(gpuTimes.begin(), gpuTimes.end());
    auto gpuMedian = gpuTimes[gpuTimes.size() / 2];
    if (gpuTimes.size() % 2 == 0) {
        gpuMedian += gpuTimes[gpuTimes.size() / 2 - 1];
        gpuMedian *= 0.5;
    }

    // Output the results.
    retval->add({
        this->data->size(),
        bboxSize,
        pipeStats.IAVertices,
        pipeStats.IAPrimitives,
        pipeStats.VSInvocations,
        pipeStats.HSInvocations,
        pipeStats.DSInvocations,
        pipeStats.GSInvocations,
        pipeStats.GSPrimitives,
        pipeStats.PSInvocations,
        gpuTimes.front(),
        gpuMedian,
        gpuTimes.back(),
        cntCpuIterations,
        cpuTime,
        static_cast<double>(cpuTime) / cntCpuIterations
        });

    return retval;
}
#endif

#if 0

/*
 * trrojan::d3d12::sphere_benchmark_base::check_data_compatibility
 */
bool trrojan::d3d12::sphere_benchmark_base::check_data_compatibility(
    const shader_id_type shaderCode) {
    if ((this->data == nullptr) || (this->data->buffer() == nullptr)) {
        log::instance().write_line(log_level::debug, "Data set is not "
            "compatible with rendering technique because no data have been "
            "loaded so far.");
        return false;
    }

    auto dataCode = this->get_data_properties(shaderCode);

    if ((shaderCode & SPHERE_TECHNIQUE_USE_SRV)
        != (dataCode & SPHERE_TECHNIQUE_USE_SRV)) {
        log::instance().write_line(log_level::debug, "Data set is not "
            "compatible with rendering technique because the technique has the "
            "shader resource view flag {} set in contrast to the data set.",
            ((shaderCode & SPHERE_TECHNIQUE_USE_SRV) != 0) ? "" : "not ");
        return false;
    }

    if (((shaderCode & SPHERE_INPUT_PV_COLOUR) != 0)
        && ((shaderCode & SPHERE_INPUT_FLT_COLOUR)
            != (dataCode & SPHERE_INPUT_FLT_COLOUR))) {
        log::instance().write_line(log_level::debug, "Data set is not "
            "compatible with rendering technique because the requested "
            "floating point conversion of colours does not match.");
        return false;
    }

    // No problem found at this point.
    return true;
}



/*
 * trrojan::d3d12::sphere_benchmark_base::get_technique
 */
trrojan::d3d12::rendering_technique &
trrojan::d3d12::sphere_benchmark_base::get_technique(Id3d12Device *device,
    shader_id_type shaderCode) {
    auto dataCode = this->get_data_properties(shaderCode);
    const auto id = shaderCode | dataCode;
    auto isFlt = ((id & SPHERE_INPUT_FLT_COLOUR) != 0);
    auto isGeo = ((id & SPHERE_TECHNIQUE_USE_GEO) != 0);
    auto isInst = ((id & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    auto isPsTex = ((id & SPHERE_INPUT_PP_INTENSITY) != 0);
    auto isRay = ((id & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    auto isSrv = ((id & SPHERE_TECHNIQUE_USE_SRV) != 0);
    auto isTess = ((id & SPHERE_TECHNIQUE_USE_TESS) != 0);
    auto isVsTex = ((id & SPHERE_INPUT_PV_INTENSITY) != 0);

    auto retval = this->technique_cache.find(id);
    if (retval == this->technique_cache.end()) {
        log::instance().write_line(log_level::verbose, "No cached sphere "
            "rendering technique for {} with data features {} (ID {}) was "
            "found. Creating a new one ...", shaderCode, dataCode, id);
        rendering_technique::input_layout_type il = nullptr;
        rendering_technique::vertex_shader_type vs = nullptr;
        rendering_technique::hull_shader_type hs = nullptr;
        rendering_technique::domain_shader_type ds = nullptr;
        rendering_technique::geometry_shader_type gs = nullptr;
        rendering_technique::pixel_shader_type ps = nullptr;
        rendering_technique::shader_resources vsRes;
        rendering_technique::shader_resources hsRes;
        rendering_technique::shader_resources dsRes;
        rendering_technique::shader_resources gsRes;
        rendering_technique::shader_resources psRes;
        auto pt = d3d12_PRIMITIVE_TOPOLOGY_POINTLIST;
        auto sid = id;

        if (!isSrv) {
            // The type of colour is only relevant for SRVs, VB-based methods do
            // not declare this in their shader flags because the layout is
            // handled via the input layout of the technique.
            sid &= ~SPHERE_INPUT_FLT_COLOUR;
        }

        auto it = this->shader_resources.find(sid);
        if (it == this->shader_resources.end()) {
            std::stringstream msg;
            msg << "Shader sources for sphere rendering method 0x"
                << std::hex << sid << " was not found." << std::ends;
            throw std::runtime_error(msg.str());
        }

        if (it->second.vertex_shader != 0) {
            auto src = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.vertex_shader), _T("SHADER"));
            vs = create_vertex_shader(device, src);
            il = create_input_layout(device, this->data->layout(), src);
        }
        if (it->second.hull_shader != 0) {
            assert(isTess);
            auto src = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.hull_shader), _T("SHADER"));
            hs = create_hull_shader(device, src);
        }
        if (it->second.domain_shader != 0) {
            assert(isTess);
            auto src = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.domain_shader), _T("SHADER"));
            ds = create_domain_shader(device, src);
        }
        if (it->second.geometry_shader != 0) {
            assert(isGeo);
            auto src = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.geometry_shader), _T("SHADER"));
            gs = create_geometry_shader(device, src);
        }
        if (it->second.pixel_shader != 0) {
            auto src = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.pixel_shader), _T("SHADER"));
            ps = create_pixel_shader(device, src);
        }


        if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
            assert(isRay);
            pt = d3d12_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            vsRes.constant_buffers.push_back(this->sphere_constants);
            vsRes.constant_buffers.push_back(this->view_constants);

            psRes.constant_buffers.push_back(this->sphere_constants);
            psRes.constant_buffers.push_back(this->view_constants);

            il = nullptr;   // Uses vertex-from-nothing technique.

#if 0
            d3d12_SO_DECLARATION_ENTRY soDecl[] = {
                { 0, "SV_POSITION", 0, 0, 4, 0 }  // Position
            };
            auto vsSrc = d3d12::plugin::load_resource(
                MAKEINTRESOURCE(it->second.vertex_shader), _T("SHADER"));
            gs = create_geometry_shader(device, vsSrc.data(), vsSrc.size(),
                soDecl, sizeof(soDecl) / sizeof(*soDecl), nullptr, 0, 0);
#endif

#if 0
            gs = create_geometry_shader(device, "C:\\Users\\mueller\\source\\repos\\TRRojan\\bin\\Debug\\PassThroughGeometryShader.cso");
#endif
        }

        if (isTess) {
            pt = d3d12_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
            vsRes.constant_buffers.push_back(this->sphere_constants);
            vsRes.constant_buffers.push_back(this->view_constants);

            hsRes.constant_buffers.push_back(nullptr);
            hsRes.constant_buffers.push_back(this->view_constants);
            hsRes.constant_buffers.push_back(this->tessellation_constants);

            dsRes.constant_buffers.push_back(nullptr);
            dsRes.constant_buffers.push_back(this->view_constants);

            psRes.constant_buffers.push_back(this->sphere_constants);
            psRes.constant_buffers.push_back(this->view_constants);
        }

        if (isGeo) {
            assert(isRay);
            pt = d3d12_PRIMITIVE_TOPOLOGY_POINTLIST;
            vsRes.constant_buffers.push_back(this->sphere_constants);
            vsRes.constant_buffers.push_back(this->view_constants);

            gsRes.constant_buffers.push_back(nullptr);
            gsRes.constant_buffers.push_back(this->view_constants);
            gsRes.constant_buffers.push_back(this->tessellation_constants);

            psRes.constant_buffers.push_back(this->sphere_constants);
            psRes.constant_buffers.push_back(this->view_constants);
        }

        if (isPsTex) {
            psRes.sampler_states.push_back(this->linear_sampler);
            rendering_technique::set_shader_resource_view(psRes,
                this->colour_map, 0);

        } else if (isVsTex) {
            vsRes.sampler_states.push_back(this->linear_sampler);
            rendering_technique::set_shader_resource_view(vsRes,
                this->colour_map, 0);
        }

        this->technique_cache[id] = rendering_technique(
            std::to_string(id), il, pt, vs, std::move(vsRes),
            hs, std::move(hsRes), ds, std::move(dsRes),
            gs, std::move(gsRes), ps, std::move(psRes));

#if 0
        {
            d3d12_RASTERIZER_DESC desc;
            rendering_technique::rasteriser_state_type state;

            ::ZeroMemory(&desc, sizeof(desc));
            desc.FillMode = d3d12_FILL_SOLID;
            desc.CullMode = d3d12_CULL_NONE;

            auto hr = device->CreateRasterizerState(&desc, &state);
            assert(SUCCEEDED(hr));

            this->technique_cache[id].set_rasteriser_state(state);
        }
#endif
    }

    retval = this->technique_cache.find(id);
    assert(retval != this->technique_cache.end());
    return retval->second;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::load_mmpld_frame
 */
void trrojan::d3d12::sphere_benchmark_base::load_mmpld_frame(Id3d12Device *dev,
    const shader_id_type shaderCode, const configuration &config) {
    auto flags = shaderCode;
    auto d = std::dynamic_pointer_cast<mmpld_data_set>(this->data);
    auto f = config.get<frame_type>(factor_frame);

    if (d == nullptr) {
        std::logic_error("A call to load_mmpld_frame is only valid while an "
            "MMPLD data set is open.");
    }

    if (config.get<bool>(factor_fit_bounding_box)) {
        flags |= mmpld_data_set::load_flag_fit_bounding_box;
    }

    log::instance().write_line(log_level::verbose, "Loading MMPLD frame {} ...",
        f);
    d->read_frame(dev, f, flags);
}
#endif
