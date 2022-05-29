// <copyright file="two_pass_volume_benchmark.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2016 - 2022 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>

#include "trrojan/d3d11/two_pass_volume_benchmark.h"

#include <DirectXMath.h>

#include <glm/ext.hpp>

#include "trrojan/log.h"

#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/utilities.h"

#include "TwoPassVolumePipeline.hlsli"
#include "volume_techniques.h"


/*
 * trrojan::d3d11::two_pass_volume_benchmark::two_pass_volume_benchmark
 */
trrojan::d3d11::two_pass_volume_benchmark::two_pass_volume_benchmark(void)
    : volume_benchmark_base("2pass-volume-renderer") { }


/*
 * trrojan::d3d11::two_pass_volume_benchmark::on_run
 */
trrojan::result trrojan::d3d11::two_pass_volume_benchmark::on_run(
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
    const auto ctx = device.d3d_context();
    const auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    std::vector<gpu_timer_type::millis_type> gpuTimes;
    auto isDisjoint = true;
    const auto minWallTime = config.get<std::uint32_t>(factor_min_wall_time);
    RaycastingConstants raycastingConstants;
    ViewConstants viewConstants;
    const auto vp = config.get<viewport_type>(factor_viewport);
    const auto volSize = this->calc_physical_volume_size();

    // Compute the bounding box in world space.
    this->calc_bounding_box(bbs, bbe);

    // If the device changed, re-create resources idependent from data and
    // viewport.
    if (contains(changed, factor_device)) {
        // Constant buffers.
        this->raycasting_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(RaycastingConstants));
        set_debug_object_name(this->raycasting_constants.p,
            "raycasting_constants");
        this->view_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(ViewConstants));
        set_debug_object_name(this->view_constants.p, "view_constants");

        // Rebuild the ray computation technique.
        {
            auto vss = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(VOLUME_RAY_PASS_VERTEX_SHADER), _T("SHADER"));
            auto vs = create_vertex_shader(dev, vss);

            ATL::CComPtr<ID3D11Buffer> ib;
            ATL::CComPtr<ID3D11Buffer> vb;
            auto ils = create_cube(dev, &vb, &ib);
            auto il = create_input_layout(dev, ils, vss);

            auto pss = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(VOLUME_RAY_PASS_PIXEL_SHADER), _T("SHADER"));
            auto ps = create_pixel_shader(dev, pss);

            auto stride = static_cast<UINT>(sizeof(DirectX::XMFLOAT3));
            this->ray_technique = rendering_technique("rays",
                { rendering_technique::vertex_buffer(vb, 8, stride) },
                rendering_technique::index_buffer(ib), il.p,
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
                vs.p, rendering_technique::shader_resources(),
                ps.p, rendering_technique::shader_resources());

            this->ray_technique.set_constant_buffers(this->view_constants,
                static_cast<rendering_technique::shader_stages>(
                rendering_technique::shader_stage::vertex));
        }

        // Add a rasteriser state without backface culling.
        {
            ATL::CComPtr<ID3D11RasterizerState> state;

            D3D11_RASTERIZER_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;

            auto hr = dev->CreateRasterizerState(&desc, &state);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr); 
            }
            this->ray_technique.set_rasteriser_state(state);
        }

        // Set an additive blending state for the ray computation pass.
        {
            ATL::CComPtr<ID3D11BlendState> state;

            D3D11_BLEND_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            for (size_t i = 0; i < 2; ++i) {
                desc.RenderTarget[i].BlendEnable = TRUE;
                desc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
                desc.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
                desc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
                desc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
                desc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[i].RenderTargetWriteMask
                    = D3D11_COLOR_WRITE_ENABLE_ALL;
            }

            auto hr = dev->CreateBlendState(&desc, &state);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            this->ray_technique.set_blend_state(state);
        }

        // Rebuild the raycasting technique.
        {
            auto src = d3d11::plugin::load_resource(MAKEINTRESOURCE(
                VOLUME_PASS_COMPUTE_SHADER), _T("SHADER"));
            auto cs = create_compute_shader(dev, src);
            auto res = rendering_technique::shader_resources();
            res.sampler_states.push_back(this->linear_sampler);
            res.resource_views.push_back(this->data_view);
            res.resource_views.push_back(this->xfer_func_view);
            res.constant_buffers.push_back(this->view_constants);
            res.constant_buffers.push_back(this->raycasting_constants);
            this->volume_technique = rendering_technique("raycasting", cs,
                std::move(res));
        }

        // Queries.
        this->done_query = create_event_query(dev);
    }

    // The data set has changed, so update SRV in the technique.
    if (contains(changed, factor_data_set)) {
        this->volume_technique.set_shader_resource_views(this->data_view,
            DATA_STAGE, 0);
    }

    // The transfer function has changed, so update the SRV in the technique.
    if (contains(changed, factor_xfer_func)) {
        this->volume_technique.set_shader_resource_views(this->xfer_func_view,
            DATA_STAGE, 1);
    }


    // If the device or the viewport changed, invalidate the intermediate
    // buffers.
    if (contains_any(changed, factor_device, factor_viewport)) {
        this->entry_source = nullptr;
        this->entry_target = nullptr;
        this->ray_source = nullptr;
        this->ray_target = nullptr;
    }

    if (this->ray_target == nullptr) {
        assert(this->entry_source == nullptr);
        assert(this->entry_target == nullptr);
        assert(this->ray_source == nullptr);
        D3D11_TEXTURE2D_DESC desc;

        ::ZeroMemory(&desc, sizeof(desc));
        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        desc.Height = vp[1];
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.Width = vp[0];

        {
            ATL::CComPtr<ID3D11Texture2D> tex;
            auto hr = dev->CreateTexture2D(&desc, nullptr, &tex);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(tex.p, "entry_target_texture");

            hr = dev->CreateRenderTargetView(tex, nullptr, &this->entry_target);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(this->entry_target.p, "entry_target_view");

            hr = dev->CreateShaderResourceView(tex, nullptr,
                &this->entry_source);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(this->entry_source.p, "entry_source_view");
        }

        {
            ATL::CComPtr<ID3D11Texture2D> tex;
            auto hr = dev->CreateTexture2D(&desc, nullptr, &tex);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(tex.p, "ray_target_texture");

            hr = dev->CreateRenderTargetView(tex, nullptr, &this->ray_target);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(this->ray_target.p, "ray_target_view");

            hr = dev->CreateShaderResourceView(tex, nullptr, &this->ray_source);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            set_debug_object_name(this->ray_source.p, "ray_source_view");
        }
    }

    // Update the raycasting parameters.
    ::ZeroMemory(&raycastingConstants, sizeof(raycastingConstants));
    raycastingConstants.ErtThreshold = config.get<float>(factor_ert_threshold);
    volume_benchmark_base::zero_is_max(raycastingConstants.ErtThreshold);

    raycastingConstants.StepLimit = config.get<std::uint32_t>(factor_max_steps);
    volume_benchmark_base::zero_is_max(raycastingConstants.StepLimit);

    raycastingConstants.StepSize = this->calc_base_step_size()
        * config.get<float>(factor_step_size);

    raycastingConstants.ImageSize.x = vp[0];
    raycastingConstants.ImageSize.y = vp[1];

    ctx->UpdateSubresource(this->raycasting_constants.p, 0, nullptr,
        &raycastingConstants, 0, 0);

    // Update the camera and view parameters.
    {
        ::ZeroMemory(&viewConstants, sizeof(viewConstants));
        const auto aspect = static_cast<float>(vp[0])
            / static_cast<float>(vp[1]);

        // First, set some basic camera parameters.
        this->camera.set_fovy(config.get<float>(factor_fovy_deg));
        this->camera.set_aspect_ratio(aspect);

        // Second, compute the current position based on the manoeuvre.
        apply_manoeuvre(this->camera, config, bbs, bbe);
        //auto hack = this->camera.get_look_from();
        //hack.z -= 50.0f;
        //this->camera.set_look_from(hack);

        // Once the camera is positioned, compute the clipping planes.
        auto clip = this->calc_clipping_planes(this->camera);
        clip.first = 0.1f;

        // Retrieve the final view parameters.
        DirectX::XMMATRIX vm, pm;
        {
            auto mat = DirectX::XMFLOAT4X4(glm::value_ptr(
                this->camera.get_projection_mx()));
            pm = DirectX::XMLoadFloat4x4(&mat);
        }
        {
            auto mat = DirectX::XMFLOAT4X4(glm::value_ptr(
                this->camera.get_view_mx()));
            vm = DirectX::XMLoadFloat4x4(&mat);
        }
        auto sm = DirectX::XMMatrixScaling(volSize[0], volSize[1], volSize[2]);

        auto eye = DirectX::XMFLOAT4(0, -(0.5f * volSize[2] + 500.0f), 0, 0);
        auto lookAt = DirectX::XMFLOAT4(0, 0, 0, 0);
        auto up = DirectX::XMFLOAT4(0, 1, 0, 0);
        //vm = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat4(&eye),
        //    DirectX::XMLoadFloat4(&lookAt), DirectX::XMLoadFloat4(&up));

        auto mvp = sm * vm * pm;
        DirectX::XMStoreFloat4x4(&viewConstants.ViewProjMatrix,
            DirectX::XMMatrixTranspose(mvp));

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

    // Determine the number of thread groups to start.
    const auto groupX = static_cast<UINT>(ceil(static_cast<float>(vp[0])
        / 16.0f));
    const auto groupY = static_cast<UINT>(ceil(static_cast<float>(vp[1])
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
                this->clear_target();
                this->begin_ray_pass(ctx, vp);
                this->ray_technique.apply(ctx);
                ctx->DrawIndexed(36, 0, 0);
                this->begin_volume_pass(ctx);
                this->volume_technique.apply(ctx);
                ctx->Dispatch(groupX, groupY, 1);
                this->present_target();
            }

            ctx->End(this->done_query);
            wait_for_event_query(ctx, this->done_query);
            batchTime = cpuTimer.elapsed_millis();

            if (batchTime < minWallTime) {
                cntPrewarms = static_cast<std::uint32_t>(std::ceil(
                    static_cast<double>(minWallTime) * cntCpuIterations
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
        this->begin_ray_pass(ctx, vp);
        this->ray_technique.apply(ctx);
        ctx->DrawIndexed(36, 0, 0);
        this->begin_volume_pass(ctx);
        this->volume_technique.apply(ctx);
        ctx->Dispatch(groupX, groupY, 1);
        this->present_target();
        gpuTimer.end(0);
        gpuTimer.end_frame();

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
        this->clear_target();
        this->begin_ray_pass(ctx, vp);
        this->ray_technique.apply(ctx);
        ctx->DrawIndexed(36, 0, 0);
        this->begin_volume_pass(ctx);
        this->volume_technique.apply(ctx);
        ctx->Dispatch(groupX, groupY, 1);
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

    return retval;
}


/*
 * trrojan::d3d11::two_pass_volume_benchmark::begin_ray_pass
 */
void trrojan::d3d11::two_pass_volume_benchmark::begin_ray_pass(
        ID3D11DeviceContext *ctx, const viewport_type& viewport) {
    assert(ctx != nullptr);
    static const float BLACK[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static ID3D11ShaderResourceView *const SRV[] = { nullptr, nullptr };

    D3D11_VIEWPORT vp;
    vp.TopLeftX = vp.TopLeftY = 0.0f;
    vp.Height = static_cast<float>(viewport[1]);
    vp.Width = static_cast<float>(viewport[0]);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    ctx->RSSetViewports(1, &vp);

    // Disable the SRVs for the render targets.
    ctx->CSSetShaderResources(first_ray_slot, 2, SRV);

    // Set the render targets.
    ID3D11RenderTargetView *rtvs[] = {
        this->entry_target.p, this->ray_target.p
    };
    ctx->ClearRenderTargetView(this->entry_target, BLACK);
    ctx->ClearRenderTargetView(this->ray_target, BLACK);
    ctx->OMSetRenderTargets(2, rtvs, nullptr);
}


/*
 * trrojan::d3d11::two_pass_volume_benchmark::begin_volume_pass
 */
void trrojan::d3d11::two_pass_volume_benchmark::begin_volume_pass(
        ID3D11DeviceContext *ctx) {
    assert(ctx != nullptr);
    static ID3D11RenderTargetView *const RTV[] = { nullptr, nullptr };
    static const auto STAGE = static_cast<rendering_technique::shader_stages>(
        rendering_technique::shader_stage::compute);

    // Enable the UAV instead of the actual render target. This will also
    // ensure that the UAV is copied to the back buffer on present if a debug
    // target is enabled.
    auto target = this->switch_to_uav_target();
    if (target != nullptr) {
        this->volume_technique.set_uavs(target, STAGE);
    }

    // Make also sure that our ray render target is not bound any more.
    ctx->OMSetRenderTargets(2, RTV, nullptr);

    // Make sure that the most recent ray texture is bound.
    this->volume_technique.set_shader_resource_views(
        { this->entry_source, this->ray_source }, STAGE, first_ray_slot);
}
