// <copyright file="cs_volume_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/cs_volume_benchmark.h"

#include <limits>
#include <memory>

#include "trrojan/io.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/stats_query.h"
#include "trrojan/d3d12/utilities.h"

#include "SinglePassVolumePipeline.hlsli"
#include "volume_techniques.h"


/*
 * trrojan::d3d12::cs_volume_benchmark::cs_volume_benchmark
 */
trrojan::d3d12::cs_volume_benchmark::cs_volume_benchmark(void)
        : volume_benchmark_base("cs-volume-renderer"),
        _ray_constants(nullptr),
        _view_constants(nullptr) { }


/*
 * trrojan::d3d12::cs_volume_benchmark::on_device_switch
 */
void trrojan::d3d12::cs_volume_benchmark::on_device_switch(device& device) {
    volume_benchmark_base::on_device_switch(device);

    // Create the buffer for the raycasting constants and map it persistently.
    // Note that we can get away with a buffer of one, because these data never
    // change throughout the benchmark run.
    this->_cb_ray = create_constant_buffer(device.d3d_device(),
        sizeof(RaycastingConstants));
    set_debug_object_name(this->_cb_ray.p, "ray_constants");

    {
        void *p;
        auto hr = this->_cb_ray->Map(0, nullptr, &p);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        this->_ray_constants = static_cast<RaycastingConstants *>(p);
    }

    // Create the buffer for the view constants and map it persistently.
    // Note that we can get away with a buffer of one, because these data never
    // change throughout the benchmark run.
    this->_cb_view = create_constant_buffer(device.d3d_device(),
        sizeof(ViewConstants));
    set_debug_object_name(this->_cb_view.p, "view_constants");

    {
        void *p;
        auto hr = this->_cb_view->Map(0, nullptr, &p);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        this->_view_constants = static_cast<ViewConstants *>(p);
    }

    // Load the shader and extract the root signature from it and use this to
    // (re-)create the PSO, which is independent from the data and the
    // configuration being run.
    {
#if defined(TRROJAN_FOR_UWP)
        const auto shader = plugin::load_shader_asset(
            "SinglePassVolumeComputeShader.cso");
#else /* defined(TRROJAN_FOR_UWP) */
        const auto shader = plugin::load_resource(
            MAKEINTRESOURCE(SINGLE_PASS_VOLUME_COMPUTE_SHADER),
            _T("SHADER"));
#endif /* defined(TRROJAN_FOR_UWP) */

        log::instance().write_line(log_level::debug, "Creating compute "
            "pipeline for single-pass compute shader ...");
        this->_compute_pipeline = create_compute_pipeline(
            this->_compute_signature, device.d3d_device(),
            shader.data(), shader.size());
        set_debug_object_name(this->_compute_signature,
            "raycast_compute_root_signature");
        set_debug_object_name(this->_compute_pipeline,
            "raycast_compute_pipeline");
    }

    // Force UAVs to be recreated. This cannot be done here, because the UAVs
    // depend on the size of the frame buffer.
    this->_uavs.clear();

    // Create a generic descriptor heap for each frame. The heap must be able to
    // hold two SRVs for the data set and transfer function, two constant
    // buffers and one UAV. Note that we can only set one heap of a kind in a
    // draw call, so we need to duplicate the data set, transfer function and
    // constants, because the UAV differs per frame.
    this->create_descriptor_heaps(device.d3d_device(), 2 + 2 + 1);
}


/*
 * trrojan::d3d12::cs_volume_benchmark::on_run
 */
trrojan::result trrojan::d3d12::cs_volume_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    // Prepare the data set, the volume meta data and the camera.
    volume_benchmark_base::on_run(device, config, power_collector, changed);

    auto cpu_iterations = static_cast<std::uint32_t>(0);
    trrojan::timer cpu_timer;
    auto dev = device.d3d_device();
    const auto gpu_freq = gpu_timer::get_timestamp_frequency(
        device.command_queue());
    const auto gpu_iterations = config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    gpu_timer gpu_timer(device.d3d_device(), 1, this->pipeline_depth());
    std::vector<gpu_timer_type::millis_type> gpu_times;
    const auto min_wall_time = config.get<std::uint32_t>(factor_min_wall_time);
    stats_query::value_type pipeline_stats;
    stats_query stats_query(device.d3d_device(), 1, 1);
    const auto volume_size = this->get_volume_resolution();
    const auto viewport = config.get<viewport_type>(factor_viewport);

    // Invalidate resources that depend on factors that have changed.
    if (contains(changed, factor_viewport)) {
        this->_uavs.clear();
    }

    // Update the camera and view parameters. As the buffers are persistently
    // mapped and we know that rendering has not yet started, we can just write
    // to the first buffer here. Theoretically, this could be optimised to only
    // update when something has changed, but we do that only once so it is
    // cheap and safer than some complex tracking logic.
    {
        assert(this->_view_constants != nullptr);
        auto& view_constants = this->_view_constants[0];
        ::ZeroMemory(&view_constants, sizeof(view_constants));

        // Retrieve the final view parameters.
        const auto dir = this->_camera.get_direction();
        const auto pos = this->_camera.get_look_from();
        const auto up = this->_camera.get_look_up();
        const auto right = glm::normalize(glm::cross(dir, up));

        view_constants.CameraDirection.x = dir.x;
        view_constants.CameraDirection.y = dir.y;
        view_constants.CameraDirection.z = dir.z;
        view_constants.CameraDirection.w = 0.0f;

        view_constants.CameraPosition.x = pos.x;
        view_constants.CameraPosition.y = pos.y;
        view_constants.CameraPosition.z = pos.z;
        view_constants.CameraPosition.w = 1.0f;

        view_constants.CameraRight.x = right.x;
        view_constants.CameraRight.y = right.y;
        view_constants.CameraRight.z = right.z;
        view_constants.CameraRight.w = 0.0f;

        view_constants.CameraUp.x = up.x;
        view_constants.CameraUp.y = up.y;
        view_constants.CameraUp.z = up.z;
        view_constants.CameraUp.w = 0.0f;

        auto clip = this->calc_clipping_planes(this->_camera);
        clip.first = 0.1f;
        view_constants.ClippingPlanes.x = clip.first;
        view_constants.ClippingPlanes.y = clip.second;

        const auto aspect = this->_camera.get_aspect_ratio();
        const auto fovy = this->_camera.get_fovy();
        view_constants.FieldOfView.y = glm::radians(fovy);
        view_constants.FieldOfView.x = aspect * view_constants.FieldOfView.y;

        const auto viewport = this->viewport();
        const auto width = static_cast<std::uint32_t>(viewport.Width);
        const auto height = static_cast<std::uint32_t>(viewport.Height);
        view_constants.ImageSize.x = width;
        view_constants.ImageSize.y = height;
    }

    // Update the raycasting parameters.
    {
        assert(this->_ray_constants != nullptr);
        auto& ray_constants = this->_ray_constants[0];
        ::ZeroMemory(&ray_constants, sizeof(ray_constants));

        ray_constants.BoxMax.x = this->_volume_bbox[1][0];
        ray_constants.BoxMax.y = this->_volume_bbox[1][1];
        ray_constants.BoxMax.z = this->_volume_bbox[1][2];
        ray_constants.BoxMax.w = 0.0f;

        ray_constants.BoxMin.x = this->_volume_bbox[0][0];
        ray_constants.BoxMin.y = this->_volume_bbox[0][1];
        ray_constants.BoxMin.z = this->_volume_bbox[0][2];
        ray_constants.BoxMin.w = 0.0f;

        ray_constants.ErtThreshold = config.get<float>(factor_ert_threshold);
        volume_benchmark_base::zero_is_max(ray_constants.ErtThreshold);

        ray_constants.StepLimit = config.get<std::uint32_t>(factor_max_steps);
        volume_benchmark_base::zero_is_max(ray_constants.StepLimit);

        ray_constants.StepSize = this->calc_base_step_size()
            * config.get<float>(factor_step_size);
    }

    // Create the UAVs for the compute shader to render to.
    if (this->_uavs.empty()) {
        log::instance().write_line(log_level::debug, "Allocating {} UAV(s) as "
            "output buffer for the compute shader ...", this->pipeline_depth());
        const auto viewport = config.get<viewport_type>(factor_viewport);

        assert(this->_descriptor_heaps.size() == this->pipeline_depth());
        this->_uavs.resize(this->pipeline_depth());

        for (std::size_t f = 0; f < this->_uavs.size(); ++f) {
            // Explicitly create the texture such that we can copy from it once
            // the compute shader finished.
            this->_uavs[f] = create_texture(device.d3d_device(),
                viewport[0],
                viewport[1],
                DXGI_FORMAT_R8G8B8A8_UNORM,
                D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS,
                D3D12_RESOURCE_STATE_COMMON);
        }
    }

    // Populate the descriptors in the descriptor heap. Theoretically, we could
    // optimise this to happen only if the data or the transfer function
    // actually changed, but this makes everything much more error-prone without
    // any performance benefits, because we do not measure this part.
    this->_descriptors.clear();

    for (std::size_t f = 0; f < this->pipeline_depth(); ++f) {
        auto heap = this->_descriptor_heaps[f].p;
        assert(heap != nullptr);
        assert(heap->GetDesc().Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        auto cpu_handle = heap->GetCPUDescriptorHandleForHeapStart();
        auto gpu_handle = heap->GetGPUDescriptorHandleForHeapStart();
        auto dev = device.d3d_device();
        const auto increment = dev->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        this->_descriptors.emplace_back();
        auto& descriptors = this->_descriptors.back();

        // t0 == data set, t1 == transfer function.
        {
            descriptors.push_back(gpu_handle);
            const auto handle_volume = cpu_handle;
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;

            descriptors.push_back(gpu_handle);
            const auto handle_xfer_func = cpu_handle;
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;

            this->set_textures(handle_volume, handle_xfer_func);
        }

        // b0 == view, b1 == raycasting
        {
            descriptors.push_back(gpu_handle);
            const auto handle_view = cpu_handle;
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;

            descriptors.push_back(gpu_handle);
            const auto handle_ray = cpu_handle;
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;

            this->set_constants(handle_view, handle_ray);
        }

        // u0 == render target
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

            descriptors.push_back(gpu_handle);
            device.d3d_device()->CreateUnorderedAccessView(this->_uavs[f],
                nullptr, &desc, cpu_handle);

            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;
        }
    }

    // Determine the size of the dispatch groups. This must be synchronised with
    // the group size in the shader itself.
    const auto groupX = static_cast<UINT>(ceil(static_cast<float>(viewport[0])
        / 16.0f));
    const auto groupY = static_cast<UINT>(ceil(static_cast<float>(viewport[1])
        / 16.0f));

#if 1
    // Record a command list for each frame for the CPU measurements.
    std::vector<ATL::CComPtr<ID3D12GraphicsCommandList>> cmd_lists(
        this->pipeline_depth());
    for (UINT i = 0; i < this->pipeline_depth(); ++i) {
        auto cmd_list = cmd_lists[i] = this->create_graphics_command_list(i);
        set_debug_object_name(cmd_list, "CPU command list #{}", i);

        cmd_list->SetComputeRootSignature(this->_compute_signature);
        cmd_list->SetPipelineState(this->_compute_pipeline);
        this->set_descriptors(cmd_list, i);

        // Enable the UAV to write to.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Dispatch the compute call.
        cmd_list->Dispatch(groupX, groupY, 1u);

        // Transition UAV to copy source.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);

        // Enable the render target as copy destination and copy the data from
        // the UAV to the back buffer.
        this->enable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);
        this->copy_to_target(cmd_list, this->_uavs[i], i);
        this->disable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);

        // Transition the UAV back to its initial state.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

        close_command_list(cmd_list);
    }
#endif

#if 1
    // Do prewarming and compute number of CPU iterations at the same time.
    log::instance().write_line(log_level::debug, "Prewarming ...");
    {
        auto batch_time = 0.0;
        auto prewarms = (std::max)(1u, config.get<unsigned int>(
            factor_min_prewarms));
        const auto wall_time = config.get<unsigned int>(factor_min_wall_time);

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

            if (batch_time < wall_time) {
                prewarms = static_cast<std::uint32_t>(std::ceil(
                    (static_cast<double>(wall_time) * cpu_iterations)
                    / batch_time));
                if (prewarms < 1) {
                    prewarms = 1;
                }
            }
        } while (batch_time < wall_time);
    }
#endif

#if 1
    // Do the wall clock measurement using the prepared command lists.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", cpu_iterations);
    const auto power_uid = benchmark_base::enter_power_scope(power_collector);
    cpu_timer.start();
    for (std::uint32_t i = 0; i < cpu_iterations; ++i) {
        auto cmd_list = cmd_lists[this->buffer_index()];
        device.execute_command_list(cmd_list);
        this->present_target();
    }
    device.wait_for_gpu();
    const auto cpu_time = cpu_timer.elapsed_millis();
    benchmark_base::leave_power_scope(power_collector);
#endif

#if 1
    // Do the GPU counter measurements using individual command lists.
    gpu_times.resize(gpu_iterations);
    for (std::size_t it = 0; it < gpu_times.size(); ++it) {
        log::instance().write_line(log_level::debug, "GPU counter measurement "
            "#{}.", it);
        const auto i = this->buffer_index();
        auto cmd_list = cmd_lists[i];
        this->reset_command_list(cmd_list);

        cmd_list->SetComputeRootSignature(this->_compute_signature);
        cmd_list->SetPipelineState(this->_compute_pipeline);
        this->set_descriptors(cmd_list, i);

        gpu_timer.start_frame();
        gpu_timer.start(cmd_list, 0);
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        cmd_list->Dispatch(groupX, groupY, 1u);

        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);

        this->enable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);
        this->copy_to_target(cmd_list, this->_uavs[i], i);
        this->disable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);

        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);
        gpu_timer.end(cmd_list, 0);
        const auto timer_index = gpu_timer.end_frame(cmd_list);

        device.close_and_execute_command_list(cmd_list);
        this->present_target();

        device.wait_for_gpu();
        gpu_times[it] = gpu_timer::to_milliseconds(
            gpu_timer.evaluate(timer_index, 0),
            gpu_freq);
    }
#endif

#if 1
    // Obtain pipeline statistics.
    log::instance().write_line(log_level::debug, "Collecting pipeline "
        "statistics ...");
    {
        const auto i = this->buffer_index();
        auto cmd_list = cmd_lists[i];
        this->reset_command_list(cmd_list);

        cmd_list->SetComputeRootSignature(this->_compute_signature);
        cmd_list->SetPipelineState(this->_compute_pipeline);
        this->set_descriptors(cmd_list, i);

        // Enable the UAV to write to.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

        // Dispatch the compute call.
        stats_query.begin(cmd_list, 0);
        cmd_list->Dispatch(groupX, groupY, 1u);
        stats_query.end(cmd_list, 0);

        // Transition UAV to copy source.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_COPY_SOURCE);

        // Enable the render target as copy destination and copy the data from
        // the UAV to the back buffer.
        this->enable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);
        this->copy_to_target(cmd_list, this->_uavs[i], i);
        this->disable_target(cmd_list, i, D3D12_RESOURCE_STATE_COPY_DEST);

        // Transition the UAV back to its initial state.
        transition_resource(cmd_list, this->_uavs[i],
            D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COMMON);

        const auto stats_index = stats_query.end_frame(cmd_list);
        device.close_and_execute_command_list(cmd_list);

        // Wait until the results are here.
        device.wait_for_gpu();

        pipeline_stats = stats_query.evaluate(stats_index, 0);
    }
#endif

#if 1
    // Compute derived statistics for GPU counters.
    const auto gpu_median = calc_median(gpu_times);
#endif

    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(std::move(config),
        std::initializer_list<std::string> {
            "benchmark",
            "power_uid",
            "data_extents",
            "gpu_time_min",
            "gpu_time_med",
            "gpu_time_max",
            "wall_time_iterations",
            "wall_time",
            "wall_time_avg",
            "cs_invocations"
    });

    // Output the results.
    retval->add({
        this->name(),
        power_uid,
        volume_size,
        gpu_times.front(),
        gpu_median,
        gpu_times.back(),
        cpu_iterations,
        cpu_time,
        static_cast<double>(cpu_time) / cpu_iterations,
        pipeline_stats.CSInvocations
    });

    return retval;
}


/*
 * trrojan::d3d12::cs_volume_benchmark::set_constants
 */
void trrojan::d3d12::cs_volume_benchmark::set_constants(
        const D3D12_CPU_DESCRIPTOR_HANDLE handle_view,
        const D3D12_CPU_DESCRIPTOR_HANDLE handle_ray) const {
    assert(this->_cb_ray != nullptr);
    assert(this->_cb_view != nullptr);
    auto device = get_device(this->_cb_ray.p);

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->_cb_ray->GetGPUVirtualAddress();
        desc.SizeInBytes = align_constant_buffer_size(
            sizeof(RaycastingConstants));
        device->CreateConstantBufferView(&desc, handle_ray);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->_cb_view->GetGPUVirtualAddress();
        desc.SizeInBytes = align_constant_buffer_size(
            sizeof(ViewConstants));
        device->CreateConstantBufferView(&desc, handle_view);
    }
}


/*
 * trrojan::d3d12::cs_volume_benchmark::set_descriptors
 */
void trrojan::d3d12::cs_volume_benchmark::set_descriptors(
        ID3D12GraphicsCommandList *cmd_list, const UINT frame) const {
    assert(cmd_list != nullptr);
    assert(this->_descriptor_heaps.size() > frame);
    cmd_list->SetDescriptorHeaps(1, &this->_descriptor_heaps[frame].p);
    cmd_list->SetComputeRootDescriptorTable(0, this->_descriptors[frame].front());
}
