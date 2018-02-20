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

#include "trrojan/d3d11/mmpld_data_set.h"
#include "trrojan/d3d11/plugin.h"
#include "trrojan/d3d11/random_sphere_data_set.h"
#include "trrojan/d3d11/utilities.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d11::sphere_benchmark::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(conservative_depth);
_SPHERE_BENCH_DEFINE_FACTOR(data_set);
_SPHERE_BENCH_DEFINE_FACTOR(edge_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(force_float_colour);
_SPHERE_BENCH_DEFINE_FACTOR(frame);
_SPHERE_BENCH_DEFINE_FACTOR(inside_tess_factor);
_SPHERE_BENCH_DEFINE_FACTOR(iterations);
_SPHERE_BENCH_DEFINE_FACTOR(method);
_SPHERE_BENCH_DEFINE_FACTOR(poly_maximum);
_SPHERE_BENCH_DEFINE_FACTOR(poly_minimum);
_SPHERE_BENCH_DEFINE_FACTOR(poly_scale);
_SPHERE_BENCH_DEFINE_FACTOR(vs_raygen);
_SPHERE_BENCH_DEFINE_FACTOR(vs_xfer_function);

#undef _SPHERE_BENCH_DEFINE_FACTOR


_DEFINE_SPHERE_TECHNIQUE_LUT(SPHERE_METHODS);


/*
 * trrojan::d3d11::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::sphere_benchmark(void)
        : benchmark_base("sphere-renderer") {
    // Declare the configuration data we need.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_conservative_depth, { true, false }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_edge_tess_factor, { std::array<float, 4> { 4, 4, 4, 4} }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_force_float_colour, { true, false }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_frame, static_cast<frame_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_inside_tess_factor, { std::array<float, 2> { 4, 4 } }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_iterations, static_cast<unsigned int>(8)));
    {
        std::vector<std::string> manifestations;
        for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr); ++i) {
            manifestations.emplace_back(::SPHERE_METHODS[i].name);
        }
        this->_default_configs.add_factor(factor::from_manifestations(
            factor_method, manifestations));
    }
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_poly_maximum, static_cast<unsigned int>(8)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_poly_minimum, static_cast<unsigned int>(4)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_poly_scale, 2.0f));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_vs_raygen, { true, false }));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_vs_xfer_function, { true, false }));

    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre, manoeuvre_type("diagonal")));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_step, static_cast<manoeuvre_step_type>(0)));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_manoeuvre_steps, static_cast<manoeuvre_step_type>(64)));

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
        const configuration& config, const std::vector<std::string>& changed) {
    typedef rendering_technique::shader_stage shader_stage;

    trrojan::timer cpuTimer;
    const auto cntIterations = config.get<int>(factor_iterations);
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    gpu_timer_type::value_type gpuFreq;
    gpu_timer_type gpuTimer;
    auto isDisjoint = true;
    auto shaderCode = sphere_benchmark::get_shader_id(config);
    SphereConstants sphereConstants;
    TessellationConstants tessConstants;
    ViewConstants viewConstants;
    D3D11_VIEWPORT viewport;

    // If the device has changed, invalidate all GPU resources and recreate the
    // data-independent ones.
    if (contains(changed, factor_device)) {
        log::instance().write_line(log_level::verbose, "Preparing GPU "
            "resources for device \"%s\" ...", device.name().c_str());
        this->data.reset();
        this->technique_cache.clear();

        // Constant buffers.
        this->sphere_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(SphereConstants));
        this->tessellation_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(TessellationConstants));
        this->view_constants = create_buffer(dev, D3D11_USAGE_DEFAULT,
            D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(ViewConstants));

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
    if (contains(changed, factor_data_set)) {
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
                "set \"%s\" ...", path.c_str());
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
            auto isFrameChanged = contains(changed, factor_frame);
            auto isFrameCompat = this->check_data_compatibility(shaderCode);

            if (isFrameChanged || !isFrameCompat) {
                this->load_mmpld_frame(dev, shaderCode, config);
            }
        }

        auto r = std::dynamic_pointer_cast<random_sphere_data_set>(this->data);
        if (r != nullptr) {
            auto isFrameCompat = this->check_data_compatibility(shaderCode);

            if (!isFrameCompat) {
                r->recreate(dev, shaderCode);
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
    auto retval = std::make_shared<basic_result>(std::move(config),
        std::initializer_list<std::string> {
            "iteration",
            "particles",
            "vs_invokes",
            "hs_invokes",
            "ds_invokes",
            "gs_invokes",
            "gs_primitives",
            "ps_invokes",
            "gpu_time",
            "wall_time"
    });

    // Compute the matrices.
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
        this->cam.set_aspect_ratio(static_cast<float>(viewport.Width)
            / static_cast<float>(viewport.Height));
        auto mat = DirectX::XMFLOAT4X4(
            glm::value_ptr(this->cam.get_projection_mx()));
        auto projection = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMStoreFloat4x4(viewConstants.ProjMatrix,
            DirectX::XMMatrixTranspose(projection));

        point_type bbs, bbe;
        this->data->bounding_box(bbs, bbe);
        auto bbSize = this->data->extents();
        auto bbMax = (std::max)(bbSize[0], bbSize[1]);
        auto dist = 0.5f * bbMax / std::tan(this->cam.get_fovy() / 180.f
            * trrojan::constants<float>::pi);

        this->cam.set_near_plane_dist(0.1f);
        this->cam.set_far_plane_dist(2.0f * bbMax);

        auto pos = glm::vec3(bbs[0]+ 0.5f * bbSize[0],
            bbs[1] + 0.5f * bbSize[1],
            bbs[2] + 0.5f * bbSize[2] + dist);
        auto lookAt = pos + glm::vec3(0.0f, 0.0f, 0.5f * bbSize[2]);

        this->cam.set_look(pos, lookAt, glm::vec3(0, 1, 0));

        this->apply_manoeuvre(this->cam, config, bbs, bbe);

        auto clipping = this->data->clipping_planes(cam);
        this->cam.set_near_plane_dist(clipping.first);
        this->cam.set_far_plane_dist(clipping.second);

        mat = DirectX::XMFLOAT4X4(glm::value_ptr(this->cam.get_view_mx()));
        auto view = DirectX::XMLoadFloat4x4(&mat);
        DirectX::XMStoreFloat4x4(viewConstants.ViewMatrix,
            DirectX::XMMatrixTranspose(view));

        auto viewDet = DirectX::XMMatrixDeterminant(view);
        auto viewInv = DirectX::XMMatrixInverse(&viewDet, view);
        DirectX::XMStoreFloat4x4(viewConstants.ViewInvMatrix,
            DirectX::XMMatrixTranspose(viewInv));

        auto viewProj = view * projection;
        DirectX::XMStoreFloat4x4(viewConstants.ViewProjMatrix,
            DirectX::XMMatrixTranspose(viewProj));

        auto viewProjDet = DirectX::XMMatrixDeterminant(viewProj);
        auto viewProjInv = DirectX::XMMatrixInverse(&viewProjDet, viewProj);
        DirectX::XMStoreFloat4x4(viewConstants.ViewProjInvMatrix,
            DirectX::XMMatrixTranspose(viewProjInv));
    }

    // Set data constants.
    {
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
            sphereConstants.GlobalRadius = 1.0f;
        }
    }

    // Set tessellation constants.
    // TODO


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
    if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
        // Instancing of quads requires 4 vertices per particle.
        cntInstances = cntPrimitives;
        cntPrimitives = 4;
    }

    // Render it.
    for (int i = 0; i < cntIterations;) {
        log::instance().write_line(log_level::debug, "Iteration %d.", i);
        cpuTimer.start();
        gpuTimer.start_frame();
        gpuTimer.start(0);
        this->clear_target();
        ctx->Begin(this->stats_query);
        if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
            ctx->DrawInstanced(cntPrimitives, cntInstances, 0, 0);
        } else {
            ctx->Draw(cntPrimitives, 0);
        }
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
            retval->add({
                i,
                this->data->size(),
                pipeStats.VSInvocations,
                pipeStats.HSInvocations,
                pipeStats.DSInvocations,
                pipeStats.GSInvocations,
                pipeStats.GSPrimitives,
                pipeStats.PSInvocations,
                gpuTime,
                cpuTime
            });
            ++i;
        }

        //this->save_target();    // TODO
    }

    return retval;
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
            "shader resource view flag %sset in contrast to the data set.",
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
    const shader_id_type FORCE_FROM_SHADER_CODE
        = SPHERE_INPUT_PV_INTENSITY
        | SPHERE_INPUT_PP_INTENSITY;

    data_properties_type retval = (this->data != nullptr)
        ? this->data->properties()
        : 0;

    if ((shaderCode & SPHERE_TECHNIQUE_USE_SRV) == 0) {
        // If no shader resource view is used, the floating point data flag is
        // not relevant. Therefore, erase it from the result.
        retval &= ~SPHERE_INPUT_FLT_COLOUR;
    }

    retval &= ~FORCE_FROM_SHADER_CODE;
    retval |= shaderCode & FORCE_FROM_SHADER_CODE;

    return retval;
}


/*
 * trrojan::d3d11::sphere_benchmark::get_technique
 */
trrojan::d3d11::rendering_technique&
trrojan::d3d11::sphere_benchmark::get_technique(ID3D11Device *device,
        const shader_id_type shaderCode) {
    auto dataCode = this->get_data_properties(shaderCode);
    auto id = shaderCode | dataCode;
    auto isPsTex = ((id & SPHERE_INPUT_PP_INTENSITY) != 0);
    auto isVsTex = ((id & SPHERE_INPUT_PV_INTENSITY) != 0);
    auto isSrv = ((id & SPHERE_TECHNIQUE_USE_SRV) != 0);
    auto isRay = ((id & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    auto isInst = ((id & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    auto isGeo = ((id & SPHERE_TECHNIQUE_USE_GEO) != 0);
    auto isTess = ((id & SPHERE_TECHNIQUE_USE_TESS) != 0);

    auto retval = this->technique_cache.find(shaderCode);
    if (retval == this->technique_cache.end()) {
        log::instance().write_line(log_level::verbose, "No cached sphere "
            "rendering technique for 0x%" PRIx64 " with data features 0x%"
            PRIx64 " (ID 0x%" PRIx64 ") was found. Creating a new one ...",
            shaderCode, dataCode, id);
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

        auto it = this->shader_resources.find(id);
        if (it == this->shader_resources.end()) {
            std::stringstream msg;
            msg << "Shader sources for sphere rendering method 0x"
                << std::hex << id << " was not found." << std::ends;
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


        if (is_technique(shaderCode, SPHERE_TECHNIQUE_QUAD_INST)) {
            assert(isRay);
            pt = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            vsRes.constant_buffers.push_back(this->sphere_constants);
            vsRes.constant_buffers.push_back(this->view_constants);

            psRes.constant_buffers.push_back(this->sphere_constants);
            psRes.constant_buffers.push_back(this->view_constants);

            il = nullptr;   // Uses vertex-from-nothing technique.
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
    }

    retval = this->technique_cache.find(id);
    return retval->second;
}


/*
 * trrojan::d3d11::sphere_benchmark::load_mmpld_frame
 */
void trrojan::d3d11::sphere_benchmark::load_mmpld_frame(ID3D11Device *dev,
        const shader_id_type shaderCode, const configuration& config) {
    auto d = std::dynamic_pointer_cast<mmpld_data_set>(this->data);
    auto f = config.get<frame_type>(factor_frame);

    if (d == nullptr) {
        std::logic_error("A call to load_mmpld_frame is only valid while an "
            "MMPLD data set is open.");
    }

    log::instance().write_line(log_level::verbose, "Loading MMPLD frame %u ...",
        f);
    d->read_frame(dev, f, shaderCode);
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
