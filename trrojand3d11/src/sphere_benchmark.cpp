/// <copyright file="sphere_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_benchmark.h"

#include <cassert>
#include <cinttypes>
#include <memory>

#include <glm/ext.hpp>

#include "trrojan/constants.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/log.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/utilities.h"

#include "SphereGeometryShader.h"
#include "SpherePipeline.hlsli"
#include "SpherePixelShader.h"
#include "SpherePixelShaderInt.h"
#include "SphereSpriteGeometryShader.h"
#include "SphereSpriteDomainShader.h"
#include "SphereSpriteHullShader.h"
#include "SphereVertexShader.h"
#include "SphereVertexShaderPvCol.h"
#include "SphereVertexShaderPvColPvRad.h"
#include "SphereVertexShaderPvInt.h"
#include "SphereVertexShaderPvIntPvRad.h"
#include "SphereVertexShaderPvRad.h"


#define _DEFINE_SHADER_PROP(v) static_cast<std::uint32_t>(v)
#define SHADER_PROP_PV_COL _DEFINE_SHADER_PROP(0x000000001)
#define SHADER_PROP_PV_RAD _DEFINE_SHADER_PROP(0x000000002)
#define SHADER_PROP_PV_INT _DEFINE_SHADER_PROP(0x000000004)
#define SHADER_PROP_VS_XFER_FUNC _DEFINE_SHADER_PROP(0x000000008)
#define SHADER_PROP_FLOAT_COL _DEFINE_SHADER_PROP(0x000000010)
#define SHADER_PROP_VPRT _DEFINE_SHADER_PROP(0x000000020)
#undef _DEFINE_SHADER_PROP

static std::map<std::string, std::map<std::uint32_t,
    trrojan::d3d11::rendering_technique>> _rendering_techniques;


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d11::sphere_benchmark::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(data_set);
_SPHERE_BENCH_DEFINE_FACTOR(frame);
_SPHERE_BENCH_DEFINE_FACTOR(iterations);
_SPHERE_BENCH_DEFINE_FACTOR(method);
_SPHERE_BENCH_DEFINE_FACTOR(vs_xfer_function);

#undef _SPHERE_BENCH_DEFINE_FACTOR


#define _SPHERE_BENCH_DEFINE_METHOD(m)                                         \
const char *trrojan::d3d11::sphere_benchmark::method_##m = #m

_SPHERE_BENCH_DEFINE_METHOD(geo_quad);
_SPHERE_BENCH_DEFINE_METHOD(geo_poly);
_SPHERE_BENCH_DEFINE_METHOD(inst_quad);
_SPHERE_BENCH_DEFINE_METHOD(geo_quad);
_SPHERE_BENCH_DEFINE_METHOD(tess_poly);
_SPHERE_BENCH_DEFINE_METHOD(tess_sphere);
_SPHERE_BENCH_DEFINE_METHOD(tess_hemisphere);

#undef _SPHERE_BENCH_DEFINE_METHOD


/*
 * trrojan::d3d11::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::sphere_benchmark(void)
        : benchmark_base("sphere-raycaster"),
        method(sphere_benchmark::method_inst_quad) {
    typedef mmpld_reader::shader_properties sp_t;

    // Define the data we need.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_frame, static_cast<frame_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_iterations, static_cast<unsigned int>(8)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_vs_xfer_function, false));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre, manoeuvre_type("diagonal")));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_step, static_cast<manoeuvre_step_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_steps, static_cast<manoeuvre_step_type>(64)));

    // Prepare a lookup table for the different rendering techniques.
#define _ADD_SPHERE_TECH(name, variant, file) this->techniques[name][variant] \
    = rendering_technique();
#undef _ADD_SPHERE_TECH
    // TODO

    // Prepare a lookup table for different variants of the pixel shader.
    this->pixel_shaders[sp_t::none] = pack_shader_source(
        ::SpherePixelShaderBytes);
    this->pixel_shaders[sp_t::intensity_xfer_function] = pack_shader_source(
        ::SpherePixelShaderIntBytes);

    // Prepare a lookup table for different variants of the vertex shader.
    this->vertex_shaders[sp_t::none] = pack_shader_source(
        ::SphereVertexShaderBytes);
    this->vertex_shaders[sp_t::intensity_xfer_function] = pack_shader_source(
        ::SphereVertexShaderPvIntBytes);
    this->vertex_shaders[sp_t::per_vertex_colour] = pack_shader_source(
        ::SphereVertexShaderPvColBytes);
    this->vertex_shaders[sp_t::per_vertex_radius] = pack_shader_source(
        ::SphereVertexShaderPvRadBytes);
    this->vertex_shaders[sp_t::intensity_xfer_function | sp_t::per_vertex_radius]
        = pack_shader_source(::SphereVertexShaderPvIntPvRadBytes);
    this->vertex_shaders[sp_t::per_vertex_colour | sp_t::per_vertex_radius]
        = pack_shader_source(::SphereVertexShaderPvColPvRadBytes);
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
        const configuration& config, const std::vector<std::string>& changed) {
    SphereConstants constants;
    trrojan::timer cpuTimer;
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    auto isDisjoint = true;
    auto isNewDevice = contains(changed, factor_device);
    auto isTessellation = false;
    D3D11_VIEWPORT viewport;

    // Determine where to perform transfer-function lookups.
    auto isVsXferFunction = config.get<bool>(factor_vs_xfer_function);

    // Retrieve the number of iterations for each frame.
    const auto cntIterations = config.get<int>(factor_iterations);

    /* Re-create data-independent GPU resources. */
    if (isNewDevice) {
        log::instance().write_line(log_level::verbose, "Preparing GPU "
            "resources for device \"%s\" ...", device.name().c_str());

        // Textures and SRVs
        this->colour_map = nullptr;
        create_viridis_colour_map(dev, &this->colour_map);

        // Constant buffers
        this->constant_buffer = create_buffer(device.d3d_device(),
            D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, nullptr,
            static_cast<UINT>(sizeof(SphereConstants)));

        // Shaders
        this->domain_shader = create_domain_shader(dev,
            ::SphereSpriteDomainShaderBytes);
        if (isTessellation) this->geometry_shader = create_geometry_shader(dev, ::SphereSpriteGeometryShaderBytes);
        else this->geometry_shader = create_geometry_shader(dev, ::SphereGeometryShaderBytes);
        this->hull_shader = create_hull_shader(dev,
            ::SphereSpriteHullShaderBytes);

        // Samplers
        this->linear_sampler = create_linear_sampler(dev);

        // Queries
        this->done_query = create_event_query(dev);
        this->stats_query = create_pipline_stats_query(dev);
    }

    /* Load the data set (header) if the file changed. */
    if (contains(changed, factor_data_set)) {
        auto path = config.get<std::string>(factor_data_set);
        log::instance().write_line(log_level::verbose, "Loading MMPLD data set "
            "\"%s\" ...", path.c_str());
        this->open_mmpld(path.c_str());
    }

    /* Read the frame. */
    if (contains_any(changed, factor_frame, factor_data_set, factor_device)) {
        auto frame = config.get<frame_type>(factor_frame);
        log::instance().write_line(log_level::verbose, "Loading MMPLD frame "
            "%u ...", frame);
        this->vertex_buffer = this->read_mmpld_frame(dev, frame);
    }

    /* Update all data-dependent resources. */
    if (contains_any(changed, factor_frame, factor_data_set, factor_device,
            factor_vs_xfer_function)) {
        auto psProps = this->get_mmpld_pixel_shader_properties(
            isVsXferFunction);
        auto psCode = this->pixel_shaders.find(psProps);
        if (psCode == this->pixel_shaders.end()) {
            throw std::runtime_error("No pixel shader is available which can "
                "fulfil the requirements of the data set.");
        }

        auto vsProps = this->get_mmpld_vertex_shader_properties(
            isVsXferFunction);
        auto vsCode = this->vertex_shaders.find(vsProps);
        if (vsCode == this->vertex_shaders.end()) {
            throw std::runtime_error("No vertex shader is available which can "
                "fulfil the requirements of the data set.");
        }

        this->input_layout = create_input_layout(device.d3d_device(),
            this->mmpld_layout, vsCode->second.first, vsCode->second.second);

        this->vertex_shader = create_vertex_shader(device.d3d_device(),
            vsCode->second.first, vsCode->second.second);

        this->pixel_shader = create_pixel_shader(device.d3d_device(),
            psCode->second.first, psCode->second.second);
    }

    /* Retrieve the viewport for rasteriser and shaders. */
    {
        auto vp = config.get<viewport_type>(factor_viewport);
        viewport.TopLeftX = viewport.TopLeftY = 0.0f;
        viewport.Width = static_cast<float>(vp[0]);
        viewport.Height = static_cast<float>(vp[1]);
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        constants.Viewport.x = 0.0f;
        constants.Viewport.y = 0.0f;
        constants.Viewport.z = viewport.Width;
        constants.Viewport.w = viewport.Height;
    }

    // Initialise the GPU timer.
    gpuTimer.initialise(dev);

    // Prepare the result set.
    auto retval = std::make_shared<basic_result>(std::move(config),
        std::initializer_list<std::string> {
            "iteration",
            "particles",
            "vs_invokes",
            "gs_invokes",
            "ps_invokes",
            "gpu_time",
            "wall_time"
    });

    /* Compute the matrices. */
    {
        //auto projection = DirectX::XMMatrixPerspectiveFovRH(std::atan(1) * 4 / 3,
        //    static_cast<float>(viewport.Width) / static_cast<float>(viewport.Height),
        //    0.1f, 10.0f);

        //auto eye = DirectX::XMFLOAT4(0, 0, 0.5f * (this->mmpld_header.bounding_box[5] - this->mmpld_header.bounding_box[2]), 0);
        //auto lookAt = DirectX::XMFLOAT4(0, 0, 0, 0);
        //auto up = DirectX::XMFLOAT4(0, 1, 0, 0);
        //auto view = DirectX::XMMatrixLookAtRH(DirectX::XMLoadFloat4(&eye),
        //    DirectX::XMLoadFloat4(&lookAt), DirectX::XMLoadFloat4(&up));

        this->cam.set_fovy(60.0f);
        this->cam.set_aspect_ratio(static_cast<float>(viewport.Width) / static_cast<float>(viewport.Height));
        auto mat = DirectX::XMFLOAT4X4(glm::value_ptr(this->cam.get_projection_mx()));
        auto projection = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMStoreFloat4x4(constants.ProjMatrix,
            DirectX::XMMatrixTranspose(projection));

        auto bbWidth = std::abs(this->mmpld_header.bounding_box[3]
            - this->mmpld_header.bounding_box[0]);
        auto bbHeight = std::abs(this->mmpld_header.bounding_box[4]
            - this->mmpld_header.bounding_box[1]);
        auto bbDepth = std::abs(this->mmpld_header.bounding_box[5]
            - this->mmpld_header.bounding_box[2]);
        auto bbMax = (std::max)(bbWidth, bbHeight);
        auto dist = 0.5f * bbMax / std::tan(this->cam.get_fovy() / 180.f
            * trrojan::constants<float>::pi);

        this->cam.set_near_plane_dist(0.1f);
        this->cam.set_far_plane_dist(2.0f * bbMax);

        auto bbStartX = std::min(this->mmpld_header.bounding_box[3],
            this->mmpld_header.bounding_box[0]);
        auto bbStartY = std::min(this->mmpld_header.bounding_box[4],
            this->mmpld_header.bounding_box[1]);
        auto bbStartZ = std::min(this->mmpld_header.bounding_box[5],
            this->mmpld_header.bounding_box[2]);

        auto pos = glm::vec3(bbStartX + 0.5f * bbWidth,
            bbStartY + 0.5f * bbHeight,
            bbStartZ + 0.5f * bbDepth + dist);
        auto lookAt = pos + glm::vec3(0.0f, 0.0f, 0.5f * bbDepth);

        this->cam.set_look(pos, lookAt, glm::vec3(0, 1, 0));

        point_type bbs, bbe;
        this->get_mmpld_bounding_box(bbs, bbe);
        this->apply_manoeuvre(this->cam, config, bbs, bbe);

        auto clipping = this->get_mmpld_clipping(cam);
        this->cam.set_near_plane_dist(clipping.first);
        this->cam.set_far_plane_dist(clipping.second);


        mat = DirectX::XMFLOAT4X4(glm::value_ptr(this->cam.get_view_mx()));
        auto view = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMStoreFloat4x4(constants.ViewMatrix,
            DirectX::XMMatrixTranspose(view));

        auto viewDet = DirectX::XMMatrixDeterminant(view);
        auto viewInv = DirectX::XMMatrixInverse(&viewDet, view);
        DirectX::XMStoreFloat4x4(constants.ViewInvMatrix,
            DirectX::XMMatrixTranspose(viewInv));

        auto viewProj = view * projection;
        DirectX::XMStoreFloat4x4(constants.ViewProjMatrix,
            DirectX::XMMatrixTranspose(viewProj));

        auto viewProjDet = DirectX::XMMatrixDeterminant(viewProj);
        auto viewProjInv = DirectX::XMMatrixInverse(&viewProjDet, viewProj);
        DirectX::XMStoreFloat4x4(constants.ViewProjInvMatrix,
            DirectX::XMMatrixTranspose(viewProjInv));
    }

    /* Compute shader constants besides matrices. */
    constants.GlobalColour.x = this->mmpld_list.colour[0];
    constants.GlobalColour.y = this->mmpld_list.colour[1];
    constants.GlobalColour.z = this->mmpld_list.colour[2];
    constants.GlobalColour.w = this->mmpld_list.colour[3];

    constants.IntensityRange.x = this->mmpld_list.min_intensity;
    constants.IntensityRange.y = this->mmpld_list.max_intensity;
    constants.GlobalRadius = this->mmpld_list.radius;
    constants.IntRangeGlobalRadTessFactor.w = 5.0f;

    /* Update constant buffer. */
    ctx->UpdateSubresource(this->constant_buffer.p, 0, nullptr,
        &constants, 0, 0);

    /* Configure vertex stage. */
    ctx->VSSetShader(this->vertex_shader.p, nullptr, 0);
    ctx->VSSetConstantBuffers(0, 1, &this->constant_buffer.p);
    ctx->VSSetShaderResources(0, 1, &this->colour_map.p);
    ctx->VSSetSamplers(0, 1, &this->linear_sampler.p);

	if (isTessellation) {
		/* Configure hull shader stage. */
		ctx->HSSetShader(this->hull_shader.p, nullptr, 0);
		ctx->HSSetConstantBuffers(0, 1, &this->constant_buffer.p);

		/* Configure domain shader stage. */
		ctx->DSSetShader(this->domain_shader.p, nullptr, 0);
		ctx->DSSetConstantBuffers(0, 1, &this->constant_buffer.p);
	} else {
        ctx->HSSetShader(nullptr, nullptr, 0);
        ctx->DSSetShader(nullptr, nullptr, 0);
	}

    if (!isTessellation) {
        /* Configure geometry stage. */
        ctx->GSSetShader(this->geometry_shader.p, nullptr, 0);
        ctx->GSSetConstantBuffers(0, 1, &this->constant_buffer.p);
    } else {
        ctx->GSSetShader(nullptr, nullptr, 0);
    }

    /* Configure pixel stage. */
    ctx->PSSetShader(this->pixel_shader.p, nullptr, 0);
    ctx->PSSetConstantBuffers(0, 1, &this->constant_buffer.p);
    ctx->PSSetShaderResources(0, 1, &this->colour_map.p);
    ctx->PSSetSamplers(0, 1, &this->linear_sampler.p);

    /* Set vertex buffer. */
    const auto offset = 0u;
    const auto stride = static_cast<UINT>(
        mmpld_reader::calc_stride(this->mmpld_list));
    const auto topology = isTessellation
        ? D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST
        : D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
    ctx->IASetVertexBuffers(0, 1, &this->vertex_buffer.p, &stride, &offset);
    ctx->IASetPrimitiveTopology(topology);
    ctx->IASetInputLayout(this->input_layout.p);

    /* Configure the rasteriser. */
    ctx->RSSetViewports(1, &viewport);
    //context->RSSetState(this->rasteriserState.Get());

    /* Render it. */
    assert(this->mmpld_list.particles <= UINT_MAX);

    for (int i = 0; i < cntIterations;) {
        log::instance().write_line(log_level::debug, "Iteration %d.", i);
        cpuTimer.start();
        gpuTimer.start_frame();
        gpuTimer.start(0);
        this->clear_target();
        ctx->Begin(this->stats_query);
        ctx->Draw(static_cast<UINT>(this->mmpld_list.particles), 0);
        ctx->End(this->stats_query);
        this->present_target();
        ctx->End(this->done_query);
        gpuTimer.end(0);
        gpuTimer.end_frame();
        wait_for_event_query(ctx, this->done_query);
        auto cpuTime = cpuTimer.elapsed_millis();

        gpuTimer.evaluate_frame(isDisjoint, gpuFreq, 5 * 1000);
        if (!isDisjoint) {
            D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeStats;
            auto hr = ctx->GetData(this->stats_query, &pipeStats, sizeof(pipeStats), 0);
            assert(SUCCEEDED(hr));

            auto gpuTime = gpu_timer_type::to_milliseconds(
                    gpuTimer.evaluate(0), gpuFreq);
            retval->add({ i,
                this->mmpld_list.particles,
                pipeStats.VSInvocations,
                pipeStats.GSInvocations,
                pipeStats.PSInvocations,
                gpuTime,
                cpuTime });
            ++i;
        }

        //this->save_target();    // TODO
    }

    return retval;
}
