// <copyright file="cs_volume_benchmark.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2016 - 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>

#include "trrojan/d3d11/cs_volume_benchmark.h"

#include <limits>
#include <memory>

#include "trrojan/log.h"

#include "trrojan/d3d11/device.h"
#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/utilities.h"

#include "SinglePassVolumePipeline.hlsli"
#include "volume_techniques.h"


/*
 * trrojan::d3d11::cs_volume_benchmark::cs_volume_benchmark
 */
trrojan::d3d11::cs_volume_benchmark::cs_volume_benchmark(void)
    : volume_benchmark_base("cs-volume-renderer") { }


/*
 * trrojan::d3d11::cs_volume_benchmark::on_run
 */
trrojan::result trrojan::d3d11::cs_volume_benchmark::on_run(
        d3d11::device& device, const configuration& config,
        const std::vector<std::string>& changed) {
    volume_benchmark_base::on_run(device, config, changed);

    static const auto DATA_STAGE = static_cast<rendering_technique::shader_stages>(
        rendering_technique::shader_stage::compute);

    glm::vec3 bbe, bbs;
    auto cntCpuIterations = static_cast<std::uint32_t>(0);
    trrojan::timer cpuTimer;
    const auto cntGpuIterations = config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    std::vector<gpu_timer_type::millis_type> gpuTimes;
    auto isDisjoint = true;
    const auto minWallTime = config.get<std::uint32_t>(factor_min_wall_time);
    RaycastingConstants raycastingConstants;
    ViewConstants viewConstants;
    const auto viewport = config.get<viewport_type>(factor_viewport);
    const auto volSize = this->calc_physical_volume_size();

    // Compute the bounding box in world space.
    this->calc_bounding_box(bbs, bbe);

    // If the device has changed, invalidate all GPU resources. Note that
    // a lot of this has already been done in the base class, eg for the
    // volume texture and the transfer function.
    if (contains(changed, factor_device)) {
        // Constant buffers.
        this->raycasting_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(RaycastingConstants));
        set_debug_object_name(this->raycasting_constants.p,
            "raycasting_constants");
        this->view_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(ViewConstants));
        set_debug_object_name(this->view_constants.p, "view_constants");

        // Rebuild the technique.
        std::vector<uint8_t> src;
#ifndef _UWP
        src = d3d11::plugin::load_resource(
            MAKEINTRESOURCE(SINGLE_PASS_VOLUME_COMPUTE_SHADER), _T("SHADER"));
#else
        auto filepath = GetAppFolder().string() + "trrojand3d11" + "/" + "d3d11" + "/" + "SinglePassVolumeComputeShader.cso";
        src = ReadFileBytes(filepath);
#endif // !_UWP
        auto cs = create_compute_shader(dev, src);
        auto res = rendering_technique::shader_resources();
        res.sampler_states.push_back(this->linear_sampler);
        res.resource_views.push_back(this->data_view);
        res.resource_views.push_back(this->xfer_func_view);
        res.constant_buffers.push_back(this->view_constants);
        res.constant_buffers.push_back(this->raycasting_constants);
        this->technique = rendering_technique("Compute shader single pass "
            "volume raycasting", cs, std::move(res));

        // Queries.
        this->done_query = create_event_query(dev);
    }

    // The data set has changed, so update SRV in the technique.
    if (contains(changed, factor_data_set)) {
        this->technique.set_shader_resource_views(this->data_view,
            DATA_STAGE, 0);
    }

    // The transfer function has changed, so update the SRV in the technique.
    if (contains(changed, factor_xfer_func)) {
        this->technique.set_shader_resource_views(this->xfer_func_view,
            DATA_STAGE, 1);
    }

    // Switch to an UAV for the compute shader. If the render target cannot
    // return an UAV, this means that the target has already been converted
    // to an UAV. In this case, we do not change anything and assume no one
    // else has changed the render target in the meantime.
    {
        auto target = this->switch_to_uav_target();
        if (target != nullptr) {
            this->technique.set_uavs(target,
                static_cast<rendering_technique::shader_stages>(
                    rendering_technique::shader_stage::compute));
        }
    }

    // Update the raycasting parameters.
    ::ZeroMemory(&raycastingConstants, sizeof(raycastingConstants));

    raycastingConstants.BoxMax.x = bbe[0];
    raycastingConstants.BoxMax.y = bbe[1];
    raycastingConstants.BoxMax.z = bbe[2];
    raycastingConstants.BoxMax.w = 0.0f;

    raycastingConstants.BoxMin.x = bbs[0];
    raycastingConstants.BoxMin.y = bbs[1];
    raycastingConstants.BoxMin.z = bbs[2];
    raycastingConstants.BoxMin.w = 0.0f;

    raycastingConstants.ErtThreshold = config.get<float>(factor_ert_threshold);
    volume_benchmark_base::zero_is_max(raycastingConstants.ErtThreshold);

    raycastingConstants.StepLimit = config.get<std::uint32_t>(factor_max_steps);
    volume_benchmark_base::zero_is_max(raycastingConstants.StepLimit);

    raycastingConstants.StepSize = this->calc_base_step_size()
        * config.get<float>(factor_step_size);
    
    ctx->UpdateSubresource(this->raycasting_constants.p, 0, nullptr,
        &raycastingConstants, 0, 0);

    // Update the camera and view parameters.
    {
        ::ZeroMemory(&viewConstants, sizeof(viewConstants));
        const auto aspect = static_cast<float>(viewport[0])
            / static_cast<float>(viewport[1]);

        // First, set some basic camera parameters.
        this->camera.set_fovy(config.get<float>(factor_fovy_deg));
        this->camera.set_aspect_ratio(aspect);

        // Second, compute the current position based on the manoeuvre.
        cs_volume_benchmark::apply_manoeuvre(this->camera, config, bbs, bbe);

        // Once the camera is positioned, compute the clipping planes.
        auto clip = this->calc_clipping_planes(this->camera);
        clip.first = 0.1f;

        // Retrieve the final view parameters.
        const auto dir = this->camera.get_direction();
        const auto pos = this->camera.get_look_from();
        const auto up = this->camera.get_look_up();
        const auto right = glm::normalize(glm::cross(dir, up));

        viewConstants.CameraDirection.x = dir.x;
        viewConstants.CameraDirection.y = dir.y;
        viewConstants.CameraDirection.z = dir.z;
        viewConstants.CameraDirection.w = 0.0f;

        viewConstants.CameraPosition.x = pos.x;
        viewConstants.CameraPosition.y = pos.y;
        viewConstants.CameraPosition.z = pos.z;
        viewConstants.CameraPosition.w = 1.0f;

        viewConstants.CameraRight.x = right.x;
        viewConstants.CameraRight.y = right.y;
        viewConstants.CameraRight.z = right.z;
        viewConstants.CameraRight.w = 0.0f;

        viewConstants.CameraUp.x = up.x;
        viewConstants.CameraUp.y = up.y;
        viewConstants.CameraUp.z = up.z;
        viewConstants.CameraUp.w = 0.0f;

        viewConstants.ClippingPlanes.x = clip.first;
        viewConstants.ClippingPlanes.y = clip.second;

        viewConstants.FieldOfView.y = glm::radians(this->camera.get_fovy());
        viewConstants.FieldOfView.x = aspect * viewConstants.FieldOfView.y;
            //= 2.0f * atan(0.5f * tan(viewConstants.FieldOfView.y) * aspect);

        viewConstants.ImageSize.x = viewport[0];
        viewConstants.ImageSize.y = viewport[1];

        // Update the GPU resources.
        ctx->UpdateSubresource(this->view_constants.p, 0, nullptr,
            &viewConstants, 0, 0);
    }

    // Initialise the GPU timer.
    gpuTimer.initialise(dev);

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

    // Activate the technique.
    this->technique.apply(ctx);

    // Determine the size of the dispatch groups. This must be synchronised with
    // the group size in the shader itself.
    const auto groupX = static_cast<UINT>(ceil(static_cast<float>(viewport[0])
        / 16.0f));
    const auto groupY = static_cast<UINT>(ceil(static_cast<float>(viewport[1])
        / 16.0f));

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
                ctx->Dispatch(groupX, groupY, 1u);
                this->present_target();
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
        ctx->Dispatch(groupX, groupY, 1u);
        gpuTimer.end(0);
        gpuTimer.end_frame();
        this->present_target();

        gpuTimer.evaluate_frame(isDisjoint, gpuFreq);
        if (!isDisjoint) {
            gpuTimes[i] = gpu_timer_type::to_milliseconds(
                gpuTimer.evaluate(0), gpuFreq);
            ++i;    // Only proceed in case of success.
        }
    }

    // Do the wall clock measurement.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", cntCpuIterations);
    cpuTimer.start();
    for (std::uint32_t i = 0; i < cntCpuIterations; ++i) {
        ctx->Dispatch(groupX, groupY, 1u);
        this->present_target();
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
        volSize,
        gpuTimes.front(),
        gpuMedian,
        gpuTimes.back(),
        cntCpuIterations,
        cpuTime,
        static_cast<double>(cpuTime) / cntCpuIterations
    });

    std::ostringstream retval_log_output;
    result_to_string(retval_log_output, *retval);
    log::instance().write_line(log_level::information, retval_log_output.str());

    return retval;
}
