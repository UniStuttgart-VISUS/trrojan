// <copyright file="sphere_benchmark_base.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_benchmark_base.h"

#include <algorithm>
#include <cinttypes>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "trrojan/constants.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/log.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

//#include "trrojan/d3d12/mmpld_data_set.h"
#include "trrojan/d3d12/plugin.h"
//#include "trrojan/d3d12/random_sphere_data_set.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::sphere_benchmark_base::factor_##f = #f

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
 * trrojan::d3d12::sphere_benchmark_base::optimise_order
 */
void trrojan::d3d12::sphere_benchmark_base::optimise_order(
        configuration_set& inOutConfs) {
    inOutConfs.optimise_order({ factor_data_set, factor_frame, factor_device });
}


/*
 * trrojan::d3d12::sphere_benchmark_base::required_factors
 */
std::vector<std::string> trrojan::d3d12::sphere_benchmark_base::required_factors(
        void) const {
    static const std::vector<std::string> retval = { factor_data_set,
        factor_device };
    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_shader_id
 */
trrojan::d3d12::sphere_benchmark_base::shader_id_type
trrojan::d3d12::sphere_benchmark_base::get_shader_id(const std::string& method) {
    for (size_t i = 0; (::SPHERE_METHODS[i].name != nullptr);++i) {
        if (method == ::SPHERE_METHODS[i].name) {
            return ::SPHERE_METHODS[i].id;
        }
    }
    /* Not found at this point. */

    return 0;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_shader_id
 */
trrojan::d3d12::sphere_benchmark_base::shader_id_type
trrojan::d3d12::sphere_benchmark_base::get_shader_id(const configuration& config) {
    const auto isConsDepth = config.get<bool>(factor_conservative_depth);
    const auto isFloat = config.get<bool>(factor_force_float_colour);
    const auto isVsRay = config.get<bool>(factor_vs_raygen);
    const auto isVsXfer = config.get<bool>(factor_vs_xfer_function);
    const auto method = config.get<std::string>(factor_method);

    auto retval = sphere_benchmark_base::get_shader_id(method);

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
 * trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants
 */
void trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants(
        TessellationConstants& out_constants, const configuration& config) {
    {
        auto f = config.get<edge_tess_factor_type>(factor_edge_tess_factor);
        out_constants.EdgeTessFactor.x = f[0];
        out_constants.EdgeTessFactor.y = f[1];
        out_constants.EdgeTessFactor.z = f[2];
        out_constants.EdgeTessFactor.w = f[3];
    }

    {
        auto f = config.get<inside_tess_factor_type>(factor_inside_tess_factor);
        out_constants.InsideTessFactor.x = f[0];
        out_constants.InsideTessFactor.y = f[1];
    }

    out_constants.AdaptiveTessMin = config.get<float>(
        factor_adapt_tess_minimum);
    out_constants.AdaptiveTessMax = config.get<float>(
        factor_adapt_tess_maximum);
    out_constants.AdaptiveTessScale = config.get<float>(
        factor_adapt_tess_scale);

    out_constants.HemisphereTessScaling = config.get<float>(
        factor_hemi_tess_scale);

    out_constants.PolygonCorners = config.get<std::uint32_t>(
        factor_poly_corners);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::is_non_float_colour
 */
bool trrojan::d3d12::sphere_benchmark_base::is_non_float_colour(
        const mmpld::colour_type colour) {
    switch (colour) {
        case mmpld::colour_type::rgb8:
        case mmpld::colour_type::rgba8:
            return true;

        default:
            return false;
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::parse_random_sphere_desc
 */
trrojan::random_sphere_generator::description
trrojan::d3d12::sphere_benchmark_base::parse_random_sphere_desc(
        const configuration& config, const shader_id_type shader_code) {
    static const auto ERASE_PROPERTIES = ~static_cast<property_mask_type>(
        property_structured_resource);

    const auto data_set = config.get<std::string>(factor_data_set);

    auto flags = static_cast<random_sphere_generator::create_flags>(
        shader_code & ERASE_PROPERTIES);
    if (config.get<bool>(factor_force_float_colour)) {
        flags |= random_sphere_generator::create_flags::float_colour;
    }

    return random_sphere_generator::parse_description(data_set, flags);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_shaders
 */
void trrojan::d3d12::sphere_benchmark_base::set_shaders(
        graphics_pipeline_builder& builder, const shader_id_type shader_id) {
#if defined(TRROJAN_FOR_UWP)
    _LOOKUP_SPHERE_SHADER_FILES(builder, shader_id,
        sphere_benchmark_base::resolve_shader_path);
#else /* defined(TRROJAN_FOR_UWP) */
    _LOOKUP_SPHERE_SHADER_RESOURCES(builder, shader_id);
#endif /* defined(TRROJAN_FOR_UWP) */
}


/*
 * trrojan::d3d12::sphere_benchmark_base::property_float_colour
 */
const trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::property_float_colour
    = SPHERE_INPUT_FLT_COLOUR;
static_assert(mmpld::particle_properties::float_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_FLT_COLOUR),
    "Constant value SPHERE_INPUT_FLT_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_benchmark_base::property_per_sphere_colour
 */
const trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::property_per_sphere_colour
    = SPHERE_INPUT_PV_COLOUR;
static_assert(mmpld::particle_properties::per_particle_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_COLOUR),
    "Constant value SPHERE_INPUT_PV_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_benchmark_base::property_per_sphere_intensity
 */
const trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::property_per_sphere_intensity
    = SPHERE_INPUT_PP_INTENSITY | SPHERE_INPUT_PV_INTENSITY;
static_assert(mmpld::particle_properties::per_particle_intensity
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_INTENSITY),
    "Constant value SPHERE_INPUT_PV_INTENSITY must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_benchmark_base::property_per_sphere_radius
 */
const trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::property_per_sphere_radius
    = SPHERE_INPUT_PV_RADIUS;
static_assert(mmpld::particle_properties::per_particle_radius
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_RADIUS),
    "Constant value SPHERE_INPUT_PV_RADIUS must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_benchmark_base::property_structured_resource
 */
const trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::property_structured_resource
    = SPHERE_TECHNIQUE_USE_SRV;


/*
 * trrojan::d3d12::sphere_benchmark_base::sphere_benchmark_base
 */
trrojan::d3d12::sphere_benchmark_base::sphere_benchmark_base(
        const std::string& name)
    : benchmark_base(name), _bbox { glm::vec3(0.0f), glm::vec3(0.0f) },
        _cnt_spheres(0),
        _colour({ 0.0f, 0.0f, 0.0f, 0.0f }),
        _data_properties(properties_type::none),
        _intensity_range({ 0.0f, 0.0f }), _max_radius(0.0f),
        _sphere_constants(nullptr), _tessellation_constants(nullptr),
        _view_constants(nullptr) {
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

    this->add_default_manoeuvre();
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_colour_map_view
 */
void trrojan::d3d12::sphere_benchmark_base::create_colour_map_view(
        ID3D12Device *device, const D3D12_CPU_DESCRIPTOR_HANDLE handle) {
    assert(device != nullptr);
    assert(this->_colour_map);
    device->CreateShaderResourceView(this->_colour_map, nullptr, handle);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_constant_buffer_view
 */
void trrojan::d3d12::sphere_benchmark_base::create_constant_buffer_view(
        ID3D12Device *device, const UINT buffer,
        const D3D12_CPU_DESCRIPTOR_HANDLE sphere_constants,
        const D3D12_CPU_DESCRIPTOR_HANDLE tessellation_constants,
        const D3D12_CPU_DESCRIPTOR_HANDLE view_constants) {
    assert(device != nullptr);
    assert(this->_constant_buffer != nullptr);

    auto address = this->_constant_buffer->GetGPUVirtualAddress();

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        desc.BufferLocation = address;
        desc.SizeInBytes = sizeof(SphereConstants);
        ::ZeroMemory(&desc, sizeof(desc));
        device->CreateConstantBufferView(&desc, sphere_constants);
    }

    address = offset_by_n<SphereConstants>(address, this->pipeline_depth());

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = address;
        desc.SizeInBytes = sizeof(TessellationConstants);
        device->CreateConstantBufferView(&desc, tessellation_constants);
    }

    address = offset_by_n<TessellationConstants>(address,
        this->pipeline_depth());

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = address;
        desc.SizeInBytes = sizeof(ViewConstants);
        device->CreateConstantBufferView(&desc, view_constants);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::fit_bounding_box
 */
void trrojan::d3d12::sphere_benchmark_base::fit_bounding_box(
    const mmpld::list_header& header, const void *particles) {
    typedef std::decay<decltype(*header.bounding_box)>::type bbox_type;
    typedef std::numeric_limits<bbox_type> bbox_limits;

    assert(particles != nullptr);
    log::instance().write_line(log_level::verbose, "Recomputing bounding "
        "box of MMPLD data from data actually contained in the active "
        "particle list ...");

    auto cur = static_cast<const std::uint8_t *>(particles);
    const auto find_radius = (header.radius <= 0.0f);
    const auto stride = mmpld::get_stride<std::size_t>(header);

    // Initialise with extrema.
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        this->_bbox[0][i] = (bbox_limits::max)();
        this->_bbox[1][i] = (bbox_limits::lowest)();
    }

    // Search minimum and maximum as well as maximum radius as necessary.
    this->_max_radius = find_radius
        ? (std::numeric_limits<decltype(header.radius)>::lowest)()
        : header.radius;
    for (std::size_t i = 0; i < header.particles; ++i, cur += stride) {
        const auto pos = reinterpret_cast<const float *>(cur);
        for (glm::length_t c = 0; c < 3; ++c) {
            if (pos[c] < this->_bbox[0][c]) {
                this->_bbox[0][c] = pos[c];
            }
            if (pos[c] > this->_bbox[1][c]) {
                this->_bbox[1][c] = pos[c];
            }
        }

        if (find_radius) {
            if (pos[4] > this->_max_radius) {
                this->_max_radius = pos[4];
            }
        }
    }

    // Account for the radius.
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        this->_bbox[0][i] -= this->_max_radius;
        this->_bbox[1][i] += this->_max_radius;
    }

    log::instance().write_line(log_level::verbose, "Recomputed "
        "single-frame bounding box of MMPLD data is ({}, {}, {}) - "
        "({}, {}, {}) with maximum radius of {}.",
        this->_bbox[0][0], this->_bbox[0][1], this->_bbox[0][2],
        this->_bbox[1][0], this->_bbox[1][1], this->_bbox[1][2],
        this->_max_radius);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_data_extents
 */
std::array<float, 3> trrojan::d3d12::sphere_benchmark_base::get_data_extents(
        void) const {
    std::array<float, 3> retval = {
        std::abs(this->_bbox[1].x - this->_bbox[0].x),
        std::abs(this->_bbox[1].y - this->_bbox[0].y),
        std::abs(this->_bbox[1].z - this->_bbox[0].z)
    };
    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_data_properties
 */
trrojan::d3d12::sphere_benchmark_base::property_mask_type
trrojan::d3d12::sphere_benchmark_base::get_data_properties(
        const shader_id_type shader_code) {
    static const shader_id_type LET_TECHNIQUE_DECIDE
        = (SPHERE_INPUT_PV_INTENSITY | SPHERE_INPUT_PP_INTENSITY);

    auto retval = static_cast<property_mask_type>(this->_data_properties);

    if ((retval & SPHERE_INPUT_PV_INTENSITY) != 0) {
        // If we need a transfer function, let the shader code decide where to
        // apply it.
        retval &= ~LET_TECHNIQUE_DECIDE;
        retval |= (shader_code & LET_TECHNIQUE_DECIDE);
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_pipeline_builder
 */
trrojan::d3d12::graphics_pipeline_builder
trrojan::d3d12::sphere_benchmark_base::get_pipeline_builder(
        const shader_id_type shader_code) {
    auto data_code = static_cast<property_mask_type>(this->get_data_properties(
        shader_code));
    const auto id = shader_code | data_code;
    const auto is_flt = ((id & SPHERE_INPUT_FLT_COLOUR) != 0);
    const auto is_geo = ((id & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_inst = ((id & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    const auto is_ps_tex = ((id & SPHERE_INPUT_PP_INTENSITY) != 0);
    const auto is_ray = ((id & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    const auto is_srv = ((id & SPHERE_TECHNIQUE_USE_SRV) != 0);
    const auto is_tess = ((id & SPHERE_TECHNIQUE_USE_TESS) != 0);
    const auto is_vs_tex = ((id & SPHERE_INPUT_PV_INTENSITY) != 0);

    graphics_pipeline_builder retval;

    // Set the shaders from the big generated lookup table.
    set_shaders(retval, id);

    // Set the input layout for techniques using VBs.
    if (!is_srv) {
        retval.set_input_layout(this->_input_layout);
    }

    retval.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        if (is_tess) {
            retval.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
        } else if (is_geo) {
            retval.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
        } else {
            retval.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        }

    } else {
        retval.set_primitive_topology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_pipeline_state
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::sphere_benchmark_base::get_pipeline_state(ID3D12Device *device,
        const shader_id_type shader_code) {
    assert(device != nullptr);
    auto data_code = static_cast<property_mask_type>(this->get_data_properties(
        shader_code));
    const auto id = shader_code | data_code;

    auto it = this->_pipeline_cache.find(id);
    auto is_create = (it == this->_pipeline_cache.end());

    if (!is_create) {
        // If the device was switched, we need to recreate the pipeline state
        // on the new device.
        auto pipeline_device = get_device(it->second);
        is_create = (pipeline_device != device);
    }

    if (is_create) {
        auto builder = this->get_pipeline_builder(shader_code);
        return this->_pipeline_cache[id] = builder.build(device);

    } else {
        return it->second;
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_sphere_constants
 */
void trrojan::d3d12::sphere_benchmark_base::get_sphere_constants(
        SphereConstants& out_constants) const {
    out_constants.GlobalColour.x = this->_colour[0];
    out_constants.GlobalColour.y = this->_colour[1];
    out_constants.GlobalColour.z = this->_colour[2];
    out_constants.GlobalColour.w = this->_colour[3];

    out_constants.GlobalRadius = this->_max_radius;

    out_constants.IntensityRange.x = this->_intensity_range[0];
    out_constants.IntensityRange.y = this->_intensity_range[1];
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_view_constants
 */
void trrojan::d3d12::sphere_benchmark_base::get_view_constants(
        ViewConstants& out_constants) const {
    auto mat = DirectX::XMFLOAT4X4(glm::value_ptr(
        this->_camera.get_projection_mx()));
    auto projection = DirectX::XMLoadFloat4x4(&mat);
    DirectX::XMStoreFloat4x4(out_constants.ProjMatrix,
        DirectX::XMMatrixTranspose(projection));

    mat = DirectX::XMFLOAT4X4(glm::value_ptr(
        this->_camera.get_view_mx()));
    auto view = DirectX::XMLoadFloat4x4(&mat);
    DirectX::XMStoreFloat4x4(out_constants.ViewMatrix,
        DirectX::XMMatrixTranspose(view));

    auto viewDet = DirectX::XMMatrixDeterminant(view);
    auto viewInv = DirectX::XMMatrixInverse(&viewDet, view);
    DirectX::XMStoreFloat4x4(out_constants.ViewInvMatrix,
        DirectX::XMMatrixTranspose(viewInv));

    auto viewProj = view * projection;
    DirectX::XMStoreFloat4x4(out_constants.ViewProjMatrix,
        DirectX::XMMatrixTranspose(viewProj));

    auto viewProjDet = DirectX::XMMatrixDeterminant(viewProj);
    auto viewProjInv = DirectX::XMMatrixInverse(&viewProjDet, viewProj);
    DirectX::XMStoreFloat4x4(out_constants.ViewProjInvMatrix,
        DirectX::XMMatrixTranspose(viewProjInv));
}


/*
 * trrojan::d3d12::sphere_benchmark_base::load_data
 */
ATL::CComPtr<ID3D12Resource> trrojan::d3d12::sphere_benchmark_base::load_data(
        ID3D12Device *device, const shader_id_type shader_code,
        const configuration& config) {
    try {
        auto desc = parse_random_sphere_desc(config, shader_code);
        this->set_properties(desc);

        void *data;
        const auto size = random_sphere_generator::create(nullptr, 0, desc);
        auto retval = create_upload_buffer(device, size);

        auto hr = retval->Map(0, nullptr, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        try {
            random_sphere_generator::create(data, size, this->_max_radius,
                desc);
            retval->Unmap(0, nullptr);
            return retval;

        } catch (...) {
            log::instance().write_line(log_level::error, "Failed to create "
                "random sphere data for specification \"{}\". The "
                "specification was correct, but the generation threw an "
                "exception.", config.get<std::string>(factor_data_set));
            retval->Unmap(0, nullptr);
            throw;
        }

    } catch (...) {
        mmpld::list_header list_header;
        const auto fit_bbox = config.get<bool>(factor_fit_bounding_box);
        const auto force_float = config.get<bool>(factor_force_float_colour);
        const auto path = config.get<std::string>(factor_data_set);
        const auto frame = config.get<frame_type>(factor_frame);

        mmpld::file<HANDLE> file(path.c_str());
        file.open_frame(frame);
        file.read_particles(false, list_header, nullptr, 0);
        this->set_properties(list_header);

        void *data;
        const auto actual_colour = list_header.colour_type;
        const auto requested_colour
            = (force_float && is_non_float_colour(list_header))
            ? mmpld::colour_type::rgba32
            : actual_colour;

        if (actual_colour != requested_colour) {
            // Patch the colour if we need to convert such that the upload
            // buffer has the correct size.
            list_header.colour_type = requested_colour;
        }

        const auto size = mmpld::get_size<UINT64>(list_header);
        auto retval = create_upload_buffer(device, size);

        auto hr = retval->Map(0, nullptr, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        try {
            if (actual_colour != requested_colour) {
                // Read into temporary buffer and convert into mapped memory.
                auto src_header = list_header;
                src_header.colour_type = actual_colour;
                std::vector<std::uint8_t> buffer(mmpld::get_size<std::size_t>(
                    src_header));
                file.read_particles(src_header, buffer.data(),
                    src_header.particles);
                mmpld::convert(buffer.data(), src_header, data, list_header);

            } else {
                // Read directly into the mapped buffer.
                file.read_particles(list_header, data, list_header.particles);
            }

            if (fit_bbox) {
                // User requested to fit bounding box to the actual data rather
                // than relying on what is in the file. Therefore, check all the
                // particles and recompute the bounding box from what we find
                // there.
                this->fit_bounding_box(list_header, data);
            } else {
                // Even if we do not recompute the bounding box, we might need
                // to check all particles to find the maximum radius for data
                // sets with varying radii.
                this->set_max_radius(list_header, data);
            }

            retval->Unmap(0, nullptr);
            return retval;

        } catch (...) {
            log::instance().write_line(log_level::error, "Failed to read "
                "MMPLD particles from \"{}\". The headers could be read, i.e."
                "the input file might be corrupted.", path);
            retval->Unmap(0, nullptr);
            throw;
        }
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::load_data_properties
 */
void trrojan::d3d12::sphere_benchmark_base::load_data_properties(
        const shader_id_type shader_code, const configuration& config) {
    try {
        auto desc = parse_random_sphere_desc(config, shader_code);
        log::instance().write_line(log_level::verbose, "Retrieving properties "
            "of random spheres \"{}\" ...",
            config.get<std::string>(factor_data_set));
        this->set_properties(desc);

    } catch (...) {
        const auto force_float = config.get<bool>(factor_force_float_colour);
        const auto frame = config.get<frame_type>(factor_frame);
        auto path = config.get<std::string>(factor_data_set);
        log::instance().write_line(log_level::verbose, "Retrieving properties "
            "of frame {0} in MMPLD data set \"{}\" ...", frame, path);
        mmpld::list_header list_header;

        mmpld::file<HANDLE> file(path.c_str());
        file.open_frame(frame);
        file.read_particles(list_header, nullptr, 0);

        if (force_float && is_non_float_colour(list_header)) {
            list_header.colour_type = mmpld::colour_type::rgba32;
        }

        this->set_properties(list_header);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::on_device_switch
 */
void trrojan::d3d12::sphere_benchmark_base::on_device_switch(
        ID3D12Device *device) {
    assert(device != nullptr);
    static constexpr auto CONSTANT_BUFFER_SIZE = sizeof(SphereConstants)
        + sizeof(TessellationConstants) + sizeof(ViewConstants);
    benchmark_base::on_device_switch(device);

    // PSOs are device-specific, so clear all cached ones.
    this->_pipeline_cache.clear();

    // Resources are device-specific, so delete and recreate them.
    this->_colour_map = create_viridis_colour_map(device);

    this->_constant_buffer = create_constant_buffer(device,
        this->pipeline_depth() * CONSTANT_BUFFER_SIZE);

    // Persistently map the upload buffer for constants.
    {
        void *p;
        auto hr = this->_constant_buffer->Map(0, nullptr, &p);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        set_debug_object_name(this->_constant_buffer.p, "constant_buffer");

        this->_sphere_constants = static_cast<SphereConstants *>(p);
        p = offset_by_n<SphereConstants>(p, this->pipeline_depth());

        this->_tessellation_constants= static_cast<TessellationConstants *>(p);
        p = offset_by_n<TessellationConstants>(p, this->pipeline_depth());

        this->_view_constants = static_cast<ViewConstants *>(p);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_clipping_planes
 */
void trrojan::d3d12::sphere_benchmark_base::set_clipping_planes(void) {
    const auto& camPos = this->_camera.get_look_from();
    const auto& view = glm::normalize(this->_camera.get_look_to() - camPos);

    auto far_plane = std::numeric_limits<float>::lowest();
    auto near_plane = (std::numeric_limits<float>::max)();

    for (auto x = 0; x < 2; ++x) {
        for (auto y = 0; y < 2; ++y) {
            for (auto z = 0; z < 2; ++z) {
                auto pt = glm::vec3(this->_bbox[x][0], this->_bbox[y][1],
                    this->_bbox[z][2]);
                auto ray = pt - camPos;
                auto dist = glm::dot(view, ray);
                if (dist < near_plane) {
                    near_plane = dist;
                }
                if (dist > far_plane) {
                    far_plane = dist;
                }
            }
        }
    }

    near_plane -= this->_max_radius;
    far_plane += this->_max_radius;

    if (near_plane < 0.0f) {
        // Plane could become negative in data set, which is illegal. A range of
        // 10k seems to be something our shaders can still handle.
        near_plane = far_plane / 10000.0f;
    }
    //near_plane = 0.01f;
    //far_plane *= 1.1f;

    log::instance().write_line(log_level::debug, "Dynamic clipping planes are "
        "located at %f and %f.", near_plane, far_plane);
    this->_camera.set_near_plane_dist(near_plane);
    this->_camera.set_far_plane_dist(far_plane);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_max_radius
 */
void trrojan::d3d12::sphere_benchmark_base::set_max_radius(
        const mmpld::list_header& header, const void *particles) {
    this->_max_radius = header.radius;

    if (this->_max_radius <= 0.0f) {
        // This list has per-particle radii, which we must check individually.
        auto cur = static_cast<const std::uint8_t *>(particles);
        const auto stride = mmpld::get_stride<std::size_t>(header);

        this->_max_radius = std::numeric_limits<
            decltype(header.radius)>::lowest();
        for (std::size_t i = 0; i < header.particles; ++i, cur += stride) {
            const auto pos = reinterpret_cast<const float *>(cur);
            if (pos[4] > this->_max_radius) {
                this->_max_radius = pos[4];
            }
        }
    }

    log::instance().write_line(log_level::verbose, "Maximum radius in MMPLD "
        "particle list was computed as {}.", this->_max_radius);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_properties
 */
void trrojan::d3d12::sphere_benchmark_base::set_properties(
        const random_sphere_generator::description& desc) {
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        this->_bbox[0][i] = -0.5 * desc.domain_size[i];
        this->_bbox[1][i] = 0.5 * desc.domain_size[i];
    }

    this->_cnt_spheres = static_cast<UINT>(desc.number);
    this->_colour[0] = 0.5f;
    this->_colour[1] = 0.5f;
    this->_colour[2] = 0.5f;
    this->_colour[3] = 1.0f;
    this->_data_properties = random_sphere_generator::get_properties(
        desc.sphere_type);
    this->_input_layout = random_sphere_generator::get_input_layout<
        D3D12_INPUT_ELEMENT_DESC>(desc.sphere_type);
    this->_intensity_range[0] = 0.0f;
    this->_intensity_range[1] = 1.0f;

    // Without the actual data, we can only set the requested maximum size, not
    // the actually realised maximum.
    this->_max_radius = desc.sphere_size[1];
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_properties
 */
void trrojan::d3d12::sphere_benchmark_base::set_properties(
        const mmpld::list_header& header) {
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        this->_bbox[0][i] = header.bounding_box[i];
        this->_bbox[1][i] = header.bounding_box[i + 3];
    }

    this->_cnt_spheres = static_cast<UINT>(header.particles);
    ::memcpy(this->_colour.data(), &header.colour, sizeof(header.colour));
    this->_data_properties = mmpld::get_properties<properties_type>(header);
    this->_input_layout = mmpld::get_input_layout<D3D12_INPUT_ELEMENT_DESC>(
        header);
    this->_intensity_range[0] = header.min_intensity;
    this->_intensity_range[1] = header.max_intensity;

    // The global radius from the list header might be wrong, but we cannot do
    // any better than this without the actual data.
    this->_max_radius = header.radius;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::update_constants
 */
void trrojan::d3d12::sphere_benchmark_base::update_constants(
        const configuration& config, const UINT buffer) {
    assert(buffer < this->pipeline_depth());
    this->get_sphere_constants(this->_sphere_constants[buffer]);
    this->get_tessellation_constants(this->_tessellation_constants[buffer], config);
    this->get_view_constants(this->_view_constants[buffer]);
}

#if 0
// If floating point colours are requested, but not available, add a
// conversion step. This step copies the first part of the particle,
// which is always the position. The following RGBA8 colour is converted
// to float4. Afterwards, the buffers are swapped such that 'data' is
// the converted buffer and the mmpld_list is updated to contain
// float4 colours.
auto forceFloat = ((options & property_float_colour) != 0);
auto notFloat = mmpld_data_set::is_non_float_colour(this->_list);
if (forceFloat && notFloat) {
    auto c = mmpld_data_set::get_colour_offset(this->_layout.begin(),
        this->_layout.end());
    assert(c != this->_layout.end());
    const auto srcOffset = c->AlignedByteOffset;
    const auto srcStride = this->stride() * this->size();
    assert(srcOffset < srcStride);

    // Recreate the header with the new colour type.
    this->_list.colour_type = mmpld_reader::colour_type::float_rgba;
    this->_layout = mmpld_data_set::get_input_layout(this->_list);
    cntData = this->stride() * this->size();

    std::vector<char> conv(cntData);
    for (size_t i = 0; i < this->_list.particles; ++i) {
        auto src = data.data() + i * srcStride;
        auto dst = conv.data() + i * this->stride();
        auto col = *reinterpret_cast<std::uint32_t *>(src + srcOffset);

        ::memcpy(dst, src, srcOffset);
        dst += this->stride();

        for (size_t c = 0; c < 4; ++i) {
            *reinterpret_cast<float *>(dst) = (col & 0xff) / 255.0f;
            col >>= 8;
            dst += sizeof(float);
        }
    }
    std::swap(data, conv);
}
#endif