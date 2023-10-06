// <copyright file="sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_benchmark.h"

#include <algorithm>
#include <cassert>
#include <cinttypes>
#include <iomanip>
#include <memory>
#include <sstream>

#include <glm/ext.hpp>

#include "trrojan/constants.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/mmpld_data_set.h"
#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/random_sphere_data_set.h"
#include "trrojan/d3d11/utilities.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d11::sphere_benchmark::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_maximum);
_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_minimum);
_SPHERE_BENCH_DEFINE_FACTOR(adapt_tess_scale);
_SPHERE_BENCH_DEFINE_FACTOR(conservative_depth);
_SPHERE_BENCH_DEFINE_FACTOR(data_set);
_SPHERE_BENCH_DEFINE_FACTOR(edge_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(fit_bounding_box);
_SPHERE_BENCH_DEFINE_FACTOR(force_float_colour);
_SPHERE_BENCH_DEFINE_FACTOR(frame);
_SPHERE_BENCH_DEFINE_FACTOR(gpu_counter_iterations);
_SPHERE_BENCH_DEFINE_FACTOR(hemi_tess_scale);
_SPHERE_BENCH_DEFINE_FACTOR(inside_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(method);
_SPHERE_BENCH_DEFINE_FACTOR(min_prewarms);
_SPHERE_BENCH_DEFINE_FACTOR(min_wall_time);
_SPHERE_BENCH_DEFINE_FACTOR(poly_corners);
_SPHERE_BENCH_DEFINE_FACTOR(vs_raygen);
_SPHERE_BENCH_DEFINE_FACTOR(vs_xfer_function);

#undef _SPHERE_BENCH_DEFINE_FACTOR


_DEFINE_SPHERE_TECHNIQUE_LUT(SPHERE_METHODS);


/*
 * trrojan::d3d11::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::sphere_benchmark(void)
        : benchmark_base("sphere-renderer") {
    // Declare the configuration data we need to have.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_adapt_tess_maximum, static_cast<unsigned int>(8)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_adapt_tess_minimum, static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_adapt_tess_scale, 2.0f));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_conservative_depth, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_edge_tess_factor, { edge_tess_factor_type { 4, 4, 4, 4} }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_fit_bounding_box, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_force_float_colour, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_frame, static_cast<frame_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_gpu_counter_iterations, static_cast<unsigned int>(7)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_hemi_tess_scale, 0.5f));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_inside_tess_factor, { inside_tess_factor_type{ 4, 4 } }));
    {
        std::vector<std::string> manifestations;
        for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr); ++i) {
            manifestations.emplace_back(::SPHERE_METHODS[i].name);
        }
        this->_default_configs.add_factor(factor::from_manifestations(
            factor_method, manifestations));
    }
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_min_prewarms, static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_min_wall_time, static_cast<unsigned int>(1000)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_poly_corners, 4u));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_vs_raygen, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_vs_xfer_function, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_sync_interval, static_cast<unsigned int>(0)));

    this->add_default_manoeuvre();

    // Build the lookup table for the shader resources.
    _ADD_SPHERE_SHADERS(this->shader_resources);
}


/*
 * trrojan::d3d11::sphere_benchmark::~sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::~sphere_benchmark(void) { }


/*
 * trrojan::d3d11::sphere_benchmark::optimise_order
 */
void trrojan::d3d11::sphere_benchmark::optimise_order(
        configuration_set& inOutConfs) {
    inOutConfs.optimise_order({ factor_data_set, factor_frame, factor_device });
}


/*
 * trrojan::d3d11::sphere_benchmark::required_factors
 */
std::vector<std::string> trrojan::d3d11::sphere_benchmark::required_factors(
        void) const {
    static const std::vector<std::string> retval = { factor_data_set,
        factor_device };
    return retval;
}


/*
 * trrojan::d3d11::sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d11::sphere_benchmark::on_run(d3d11::device& device,
        const configuration& config,
        power_collector::pointer& powerCollector,
        const std::vector<std::string>& changed) {
    typedef rendering_technique::shader_stage shader_stage;

    std::array<float, 3> bboxSize;
    trrojan::timer cpuTimer;
    auto cntCpuIterations = static_cast<std::uint32_t>(0);
    const auto cntGpuIterations= config.get<std::uint32_t>(
        factor_gpu_counter_iterations);
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    std::vector<gpu_timer_type::millis_type> gpuTimes;
    auto isDisjoint = true;
    const auto minWallTime = config.get<std::uint32_t>(factor_min_wall_time);
    D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeStats;
    std::string powerUid;
    auto shaderCode = sphere_benchmark::get_shader_id(config);
    SphereConstants sphereConstants;
    TessellationConstants tessConstants;
    ViewConstants viewConstants;
    D3D11_VIEWPORT viewport;

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
        this->sphere_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(SphereConstants));
        set_debug_object_name(this->sphere_constants.p, "sphere_constants");
        this->tessellation_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(TessellationConstants));
        set_debug_object_name(this->tessellation_constants.p,
            "tessellation_constants");
        this->view_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(ViewConstants));
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
        } catch (std::exception& ex) {
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
    auto& technique = this->get_technique(dev, shaderCode);
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
    auto retval = std::make_shared<basic_result>(config,
        std::initializer_list<std::string> {
            "benchmark",
            "power_uid",
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

    // Set data constants. Note that his must be done before computing the
    // matrices because we will use sphereConstants.GlobalRadius there.
    {
        ::ZeroMemory(&sphereConstants, sizeof(SphereConstants));

        auto m = std::dynamic_pointer_cast<mmpld_data_set>(this->data);
        if (m != nullptr) {
            auto& l = m->header();

            sphereConstants.GlobalColour.x = l.colour[0];
            sphereConstants.GlobalColour.y = l.colour[1];
            sphereConstants.GlobalColour.z = l.colour[2];
            sphereConstants.GlobalColour.w = l.colour[3];

            sphereConstants.IntensityRange.x = l.min_intensity;
            sphereConstants.IntensityRange.y = l.max_intensity;
            sphereConstants.GlobalRadius = l.radius;

        } else {
            sphereConstants.GlobalColour.x = 0.5f;
            sphereConstants.GlobalColour.y = 0.5f;
            sphereConstants.GlobalColour.z = 0.5f;
            sphereConstants.GlobalColour.w = 1.f;

            sphereConstants.IntensityRange.x = 0.0f;
            sphereConstants.IntensityRange.y = 1.0f;
            sphereConstants.GlobalRadius = this->data->max_radius();
            // Note: the maximum radius is the global radius for random
            // spheres without per-sphere radius.
        }
    }

    // Compute the matrices.
    {
        const auto aspect = static_cast<float>(viewport.Width)
            / static_cast<float>(viewport.Height);
        const auto fovyDeg = 60.0f;
        point_type bbs, bbe;

        // Retrieve the bounding box of the data.
        this->data->bounding_box(bbs, bbe);
        for (size_t i = 0; i < 3; ++i) {
            bboxSize[i] = std::abs(bbe[i] - bbs[1]);
        }

        // Set basic view parameters.
        this->cam.set_fovy(fovyDeg);
        this->cam.set_aspect_ratio(aspect);

        // Apply the current step of the manoeuvre.
        sphere_benchmark::apply_manoeuvre(this->cam, config, bbs, bbe);

        // Compute the clipping planes based on the current view.
        const auto clipping = this->data->clipping_planes(cam,
            sphereConstants.GlobalRadius);
        this->cam.set_near_plane_dist(clipping.first);
        this->cam.set_far_plane_dist(clipping.second);

        // Retrieve the matrices.
        auto projection = this->cam.calc_projection_mxz0();
        viewConstants.ProjMatrix[0] = DirectX::XMFLOAT4X4(
            glm::value_ptr(projection));

        auto &view = this->cam.get_view_mx();
        viewConstants.ViewMatrix[0] = DirectX::XMFLOAT4X4(
            glm::value_ptr(view));

        auto viewInv = glm::inverse(view);
        viewConstants.ViewInvMatrix[0] = DirectX::XMFLOAT4X4(
            glm::value_ptr(viewInv));

        auto viewProj = projection * view;
        viewConstants.ViewProjMatrix[0] = DirectX::XMFLOAT4X4(
            glm::value_ptr(viewProj));

        auto viewProjInv = glm::inverse(viewProj);
        viewConstants.ViewProjInvMatrix[0] = DirectX::XMFLOAT4X4(
            glm::value_ptr(viewProjInv));
    }

    // Set tessellation constants.
    {
        auto f = config.get<edge_tess_factor_type>(factor_edge_tess_factor);
        tessConstants.EdgeTessFactor.x = f[0];
        tessConstants.EdgeTessFactor.y = f[1];
        tessConstants.EdgeTessFactor.z = f[2];
        tessConstants.EdgeTessFactor.w = f[3];
    }

    {
        auto f = config.get<inside_tess_factor_type>(factor_inside_tess_factor);
        tessConstants.InsideTessFactor.x = f[0];
        tessConstants.InsideTessFactor.y = f[1];
    }

    tessConstants.AdaptiveTessMin = config.get<float>(
        factor_adapt_tess_minimum);
    tessConstants.AdaptiveTessMax = config.get<float>(
        factor_adapt_tess_maximum);
    tessConstants.AdaptiveTessScale = config.get<float>(
        factor_adapt_tess_scale);

    tessConstants.HemisphereTessScaling = config.get<float>(
        factor_hemi_tess_scale);

    tessConstants.PolygonCorners = config.get<std::uint32_t>(
        factor_poly_corners);

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
        auto soBuffer = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_STREAM_OUTPUT, nullptr, 4000 * sizeof(float) * 4, 0);
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
                this->present_target(config);
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
        if (isInstanced) {
            ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
        } else {
            ctx->Draw(cntPrimitives, 0);
        }
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
    this->present_target(0);
#if defined(CREATE_D2D_OVERLAY)
    // Using the overlay will change the state such that we need to
    // re-apply it after presenting.
    technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
    wait_for_stats_query(pipeStats, ctx, this->stats_query);

    // Do the wall clock measurement.
    log::instance().write_line(log_level::debug, "Measuring wall clock "
        "timings over {} iterations ...", cntCpuIterations);
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    if (powerCollector != nullptr) {
        // If we have a power sensor, we want to record data now.
        powerUid = powerCollector->set_next_unique_description();
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

    cpuTimer.start();
    for (std::uint32_t i = 0; i < cntCpuIterations; ++i) {
        this->clear_target();
        if (isInstanced) {
            ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
        } else {
            ctx->Draw(cntPrimitives, 0);
        }
        this->present_target(config);
#if defined(CREATE_D2D_OVERLAY)
        // Using the overlay will change the state such that we need to
        // re-apply it after presenting.
        technique.apply(ctx);
#endif /* defined(CREATE_D2D_OVERLAY) */
    }
    ctx->End(this->done_query);
    wait_for_event_query(ctx, this->done_query);
    auto cpuTime = cpuTimer.elapsed_millis();

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    if (powerCollector != nullptr) {
        // Commit power samples and prevent collection until next wall-clock
        // measurement cycle.
        powerCollector->set_description("");
    }
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */

    // Compute derived statistics for GPU counters.
    std::sort(gpuTimes.begin(), gpuTimes.end());
    auto gpuMedian = gpuTimes[gpuTimes.size() / 2];
    if (gpuTimes.size() % 2 == 0) {
        gpuMedian += gpuTimes[gpuTimes.size() / 2 - 1];
        gpuMedian *= 0.5;
    }

    // Output the results.
    retval->add({
        this->name(),
        powerUid,
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


/*
 * trrojan::d3d11::sphere_benchmark::get_shader_file_id
 */
std::string trrojan::d3d11::sphere_benchmark::get_shader_file_id(
        const shader_id_type id) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(16) << std::hex << id;
    return ss.str();
}


/*
 * trrojan::d3d11::sphere_benchmark::get_shader_id
 */
trrojan::d3d11::sphere_benchmark::shader_id_type
trrojan::d3d11::sphere_benchmark::get_shader_id(const std::string& method) {
    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr);++i) {
        if (method == ::SPHERE_METHODS[i].name) {
            return ::SPHERE_METHODS[i].id;
        }
    }
    /* Not found at this point. */

    return 0;
}


/*
 * trrojan::d3d11::sphere_benchmark::get_shader_id
 */
trrojan::d3d11::sphere_benchmark::shader_id_type
trrojan::d3d11::sphere_benchmark::get_shader_id(const configuration& config) {
    const auto isConsDepth = config.get<bool>(factor_conservative_depth);
    const auto isFloat = config.get<bool>(factor_force_float_colour);
    const auto isVsRay = config.get<bool>(factor_vs_raygen);
    const auto isVsXfer = config.get<bool>(factor_vs_xfer_function);
    const auto method = config.get<std::string>(factor_method);

    auto retval = sphere_benchmark::get_shader_id(method);

    if ((retval & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0) {
        // If the shader is using raycasting, add special flags relevant only
        // for raycasting.
        if (isConsDepth) {
            retval |= SPHERE_VARIANT_CONSERVATIVE_DEPTH;
        }
        if (isVsRay) {
            retval |= SPHERE_VARIANT_PV_RAY;
        }
    }

    if ((retval & SPHERE_TECHNIQUE_USE_SRV) != 0) {
        // If the shader is using a shader resource view for the data, the
        // floating point conversion flag is relevant.
        if (isFloat) {
            retval |= SPHERE_INPUT_FLT_COLOUR;
        }
    }

    // Set the location of the transfer function lookup unconditionally,
    // because we do not know whether the data could require this. We need to
    // erase this flag later if the data do not need it.
    if (isVsXfer) {
        retval |= SPHERE_INPUT_PV_INTENSITY;
    } else {
        retval |= SPHERE_INPUT_PP_INTENSITY;
    }

    return retval;
}


/*
 * trrojan::d3d11::sphere_benchmark::check_data_compatibility
 */
bool trrojan::d3d11::sphere_benchmark::check_data_compatibility(
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
 * trrojan::d3d11::sphere_benchmark::get_data_properties
 */
trrojan::d3d11::sphere_benchmark::data_properties_type
trrojan::d3d11::sphere_benchmark::get_data_properties(
        const shader_id_type shaderCode) {
    data_properties_type retval = (this->data != nullptr)
        ? this->data->properties()
        : 0;

    if ((retval & SPHERE_INPUT_PV_INTENSITY) != 0) {
        // If we need a transfer function, let the shader code decide where to
        // apply it.
        assert((retval & SPHERE_INPUT_PP_INTENSITY) != 0);
        const shader_id_type LET_TECHNIQUE_DECIDE
            = SPHERE_INPUT_PV_INTENSITY
            | SPHERE_INPUT_PP_INTENSITY;
        retval &= ~LET_TECHNIQUE_DECIDE;
        retval |= (shaderCode & LET_TECHNIQUE_DECIDE);
    }

    return retval;
}


/*
 * trrojan::d3d11::sphere_benchmark::get_technique
 */
trrojan::d3d11::rendering_technique&
trrojan::d3d11::sphere_benchmark::get_technique(ID3D11Device *device,
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
        auto pt = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
        auto sid = id;

        if (!isSrv) {
            // The type of colour is only relevant for SRVs, VB-based methods do
            // not declare this in their shader flags because the layout is
            // handled via the input layout of the technique.
            sid &= ~SPHERE_INPUT_FLT_COLOUR;
        }

#if defined(TRROJAN_FOR_UWP)
        // TODO: this is only prepared, probably not working (read_binary_file) out of the box.
        const auto base_path = plugin::get_directory()
            + directory_separator_char;
        const auto fid_ext = get_shader_file_id(sid) + ".cso";

        {
            auto path = base_path + "SphereVertexShader" + fid_ext;
            auto src = read_binary_file(path);
            vs = create_vertex_shader(device, src);
            il = create_input_layout(device, this->data->layout(), src);
        }

        if (isTess) {
            {
                auto path = base_path + "SphereHullShader" + fid_ext;
                auto src = read_binary_file(path);
                hs = create_hull_shader(device, src);
            }

            {
                auto path = base_path + "SphereDomainShader" + fid_ext;
                auto src = read_binary_file(path);
                ds = create_domain_shader(device, src);
            }
        }

        if (isGeo) {
            auto path = base_path + "SphereGeometryShader" + fid_ext;
            auto src = read_binary_file(path);
            gs = create_geometry_shader(device, src);
        }

        {
            auto path = base_path + "SpherePixelShader" + fid_ext;
            auto src = read_binary_file(path);
            ps = create_pixel_shader(device, src);
        }

#else /* defined(TRROJAN_FOR_UWP) */
        auto it = this->shader_resources.find(sid);
        if (it == this->shader_resources.end()) {
            std::stringstream msg;
            msg << "Shader sources for sphere rendering method 0x"
                << std::hex << sid << " was not found." << std::ends;
            throw std::runtime_error(msg.str());
        }

        if (it->second.vertex_shader != 0) {
            auto src = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.vertex_shader), _T("SHADER"));
            vs = create_vertex_shader(device, src);
            il = create_input_layout(device, this->data->layout(), src);
        }
        if (it->second.hull_shader != 0) {
            assert(isTess);
            auto src = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.hull_shader), _T("SHADER"));
            hs = create_hull_shader(device, src);
        }
        if (it->second.domain_shader != 0) {
            assert(isTess);
            auto src = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.domain_shader), _T("SHADER"));
            ds = create_domain_shader(device, src);
        }
        if (it->second.geometry_shader != 0) {
            assert(isGeo);
            auto src = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.geometry_shader), _T("SHADER"));
            gs = create_geometry_shader(device, src);
        }
        if (it->second.pixel_shader != 0) {
            auto src = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.pixel_shader), _T("SHADER"));
            ps = create_pixel_shader(device, src);
        }
#endif /* defined(TRROJAN_FOR_UWP) */


        if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
            assert(isRay);
            pt = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            vsRes.constant_buffers.push_back(this->sphere_constants);
            vsRes.constant_buffers.push_back(this->view_constants);

            psRes.constant_buffers.push_back(this->sphere_constants);
            psRes.constant_buffers.push_back(this->view_constants);

            il = nullptr;   // Uses vertex-from-nothing technique.

#if 0
            D3D11_SO_DECLARATION_ENTRY soDecl[] = {
                { 0, "SV_POSITION", 0, 0, 4, 0 }  // Position
            };
            auto vsSrc = d3d11::plugin::load_resource(
                MAKEINTRESOURCE(it->second.vertex_shader), _T("SHADER"));
            gs = create_geometry_shader(device, vsSrc.data(), vsSrc.size(),
                soDecl, sizeof(soDecl) / sizeof(*soDecl), nullptr, 0, 0);
#endif

#if 0
            gs = create_geometry_shader(device, "C:\\Users\\mueller\\source\\repos\\TRRojan\\bin\\Debug\\PassThroughGeometryShader.cso");
#endif
        }

        if (isTess) {
            pt = D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
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
            pt = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
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
            D3D11_RASTERIZER_DESC desc;
            rendering_technique::rasteriser_state_type state;

            ::ZeroMemory(&desc, sizeof(desc));
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;

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
 * trrojan::d3d11::sphere_benchmark::load_mmpld_frame
 */
void trrojan::d3d11::sphere_benchmark::load_mmpld_frame(ID3D11Device *dev,
        const shader_id_type shaderCode, const configuration& config) {
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


/*
 * trrojan::d3d11::sphere_benchmark::make_random_spheres
 */
void trrojan::d3d11::sphere_benchmark::make_random_spheres(
        ID3D11Device *dev, const shader_id_type shaderCode,
        const configuration& config) {
    auto conf = config.get<std::string>(factor_data_set);
    auto flags = static_cast<random_sphere_data_set::create_flags>(shaderCode);
    auto forceFloat = config.get<bool>(factor_force_float_colour);

    if (forceFloat) {
        flags |= random_sphere_data_set::property_float_colour;
    }

    this->data = random_sphere_data_set::create(dev, flags, conf);
}
