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

#include "trrojan/clipping.h"
#include "trrojan/constants.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/log.h"
//#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/plugin.h"
#include "trrojan/d3d12/sphere_benchmark.h"
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
 * trrojan::d3d12::sphere_benchmark_base::get_primitive_topology
 */
D3D12_PRIMITIVE_TOPOLOGY
trrojan::d3d12::sphere_benchmark_base::get_primitive_topology(
        const shader_id_type shader_code) {
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);

    if (is_geo) {
        return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    }

    if (is_tess) {
        return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
    }

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    }

    return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_primitive_topology_type
 */
D3D12_PRIMITIVE_TOPOLOGY_TYPE
trrojan::d3d12::sphere_benchmark_base::get_primitive_topology_type(
        const shader_id_type shader_code) {
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);

    if (is_geo) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    }

    if (is_tess) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    }

    if (is_technique(shader_code, SPHERE_TECHNIQUE_QUAD_INST)) {
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }

    return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
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
        // floating point conversion flag is relevant. Note that this flag
        // set speculatively and might need to be removed if the data does
        // not contain per-sphere colours.
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
 * trrojan::d3d12::sphere_benchmark_base::get_shader_id
 */
trrojan::d3d12::sphere_benchmark_base::shader_id_type
trrojan::d3d12::sphere_benchmark_base::get_shader_id(shader_id_type shader_code,
        property_mask_type data_code) {
    // If there are no per-sphere colours, erase the float flag if it has been
    // requested by the configuration.
    if ((data_code & SPHERE_INPUT_PV_COLOUR) == 0) {
        shader_code &= ~SPHERE_INPUT_FLT_COLOUR;
        data_code &= ~SPHERE_INPUT_FLT_COLOUR;
    }

    // We create only shader code for floating-point conversion if the shader
    // uses an SRV. In all other cases, the floating-point colour flag must be
    // erased.
    if ((shader_code & SPHERE_TECHNIQUE_USE_SRV) == 0) {
        shader_code &= ~SPHERE_INPUT_FLT_COLOUR;
        data_code &= ~SPHERE_INPUT_FLT_COLOUR;
    }

    // If the data does not a per-sphere intensity, erase the texture lookup for
    // vertex and pixel shader. If the data does contain per-sphere intensity
    // data, erase it from there such that the shader code determines where the
    // intensity is converted into colour.
    if ((data_code & property_per_sphere_intensity) == 0) {
        shader_code &= ~static_cast<shader_id_type>(property_per_sphere_intensity);
    } else {
        data_code &= ~property_per_sphere_intensity;
    }

    // Finally, merge any data-dependent flags, eg floating-point colours from
    // the data code.
    shader_code |= data_code;

#if (defined(DEBUG) || defined(_DEBUG))
    const auto is_col = ((shader_code & SPHERE_INPUT_PV_COLOUR) != 0);
    const auto is_flt = ((shader_code & SPHERE_INPUT_FLT_COLOUR) != 0);
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_inst = ((shader_code & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    const auto is_ps_tex = ((shader_code & SPHERE_INPUT_PP_INTENSITY) != 0);
    const auto is_ray = ((shader_code & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    const auto is_srv = ((shader_code & SPHERE_TECHNIQUE_USE_SRV) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);
    const auto is_vs_tex = ((shader_code & SPHERE_INPUT_PV_INTENSITY) != 0);

    log::instance().write_line(log_level::debug, "Shader code 0x{:x}: "
        "colour = {}, float colour = {}, geometry = {}, instancing = {}, "
        "psxfer = {}, raycasting = {}, srv = {}, tessellation = {}, "
        "vsxfer = {}.", shader_code, is_col, is_flt, is_geo, is_inst,
        is_ps_tex, is_ray, is_srv, is_tess, is_vs_tex);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

    return shader_code;
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
 * trrojan::d3d12::sphere_benchmark_base::set_descriptors
 */
void trrojan::d3d12::sphere_benchmark_base::set_descriptors(
        ID3D12GraphicsCommandList *cmd_list,
        const descriptor_table_type& descriptors) {
    assert(cmd_list != nullptr);
    cmd_list->SetDescriptorHeaps(1, &descriptors.first.p);

    for (UINT i = 0; i < descriptors.second.size(); ++i) {
        cmd_list->SetGraphicsRootDescriptorTable(i,
            descriptors.second[i]);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_shaders
 */
void trrojan::d3d12::sphere_benchmark_base::set_shaders(
        graphics_pipeline_builder& builder, const shader_id_type shader_id) {
    //const auto is_col = ((shader_id & SPHERE_INPUT_PV_COLOUR) != 0);
    builder.reset_shaders();
#if defined(_UWP)
    _LOOKUP_SPHERE_SHADER_FILES(builder, shader_id,
        sphere_benchmark_base::resolve_shader_path);
#else /* defined(_UWP) */
    _LOOKUP_SPHERE_SHADER_RESOURCES(builder, shader_id);
#endif /* defined(_UWP) */
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
        _cnt_spheres(0), _cnt_descriptor_tables(0),
        _colour({ 0.0f, 0.0f, 0.0f, 0.0f }),
        _data_properties(properties_type::none),
        _intensity_range({ 0.0f, 0.0f }), _max_radius(0.0f),
        _sphere_constants(nullptr), _stride(0),
        _tessellation_constants(nullptr), _view_constants(nullptr) {
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
        factor_inside_tess_factor, { inside_tess_factor_type { 4, 4 } }));
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
 * trrojan::d3d12::sphere_benchmark_base::set_clipping_planes
 */
void trrojan::d3d12::sphere_benchmark_base::configure_camera(
        const configuration& config, const float fovy) {
    this->set_aspect_from_viewport(this->_camera);
    this->_camera.set_fovy(fovy);
    sphere_benchmark::apply_manoeuvre(this->_camera, config,
        this->get_bbox_start(), this->get_bbox_end());
    trrojan::set_clipping_planes(this->_camera, this->_bbox, this->_max_radius);
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
        const D3D12_CPU_DESCRIPTOR_HANDLE view_constants,
        const D3D12_CPU_DESCRIPTOR_HANDLE tessellation_constants) {
    assert(device != nullptr);

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_sphere_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(sizeof(SphereConstants));
        device->CreateConstantBufferView(&desc, sphere_constants);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_tessellation_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(
            sizeof(TessellationConstants));
        device->CreateConstantBufferView(&desc, tessellation_constants);
    }

    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.BufferLocation = this->get_view_constants(buffer);
        desc.SizeInBytes = align_constant_buffer_size(sizeof(ViewConstants));
        device->CreateConstantBufferView(&desc, view_constants);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::create_descriptor_heaps
 */
void trrojan::d3d12::sphere_benchmark_base::create_descriptor_heaps(
        ID3D12Device *device, const shader_id_type shader_code) {
    // Three constant buffers are always required.
    this->_cnt_descriptor_tables = 3;

    if (is_any_technique(shader_code, SPHERE_INPUT_PV_INTENSITY
            | SPHERE_TECHNIQUE_USE_INSTANCING)) {
        // Note: we use a little hack that the instancing data are always in t1,
        // even if there is no colour map bound to t0. This makes the allocation
        // here a bit easier as it is always the same size ...
        this->_cnt_descriptor_tables += 2;
    }

    if (is_any_technique(shader_code, SPHERE_INPUT_PP_INTENSITY)) {
        ++this->_cnt_descriptor_tables;
    }

    // Add one slot for the root descriptor table.
    ++this->_cnt_descriptor_tables;

    this->create_descriptor_heaps(device, this->_cnt_descriptor_tables);
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
        // If we need a transfer function, which is identified by the per-vertex
        // intensity flag, let the shader code decide which of the flags to use,
        // per-vertex or per-pixel, by erasing the indicator for per-vertex
        // intensity and copying the one from the shader.
        retval &= ~SPHERE_INPUT_PV_INTENSITY;
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
    const auto is_flt = ((shader_code & SPHERE_INPUT_FLT_COLOUR) != 0);
    const auto is_geo = ((shader_code & SPHERE_TECHNIQUE_USE_GEO) != 0);
    const auto is_inst = ((shader_code & SPHERE_TECHNIQUE_USE_INSTANCING) != 0);
    const auto is_ps_tex = ((shader_code & SPHERE_INPUT_PP_INTENSITY) != 0);
    const auto is_ray = ((shader_code & SPHERE_TECHNIQUE_USE_RAYCASTING) != 0);
    const auto is_srv = ((shader_code & SPHERE_TECHNIQUE_USE_SRV) != 0);
    const auto is_tess = ((shader_code & SPHERE_TECHNIQUE_USE_TESS) != 0);
    const auto is_vs_tex = ((shader_code & SPHERE_INPUT_PV_INTENSITY) != 0);

    graphics_pipeline_builder retval;

    // Set the shaders from the big generated lookup table.
    set_shaders(retval, shader_code);

    // Set the input layout for techniques using VBs.
    if (!is_srv) {
        retval.set_input_layout(this->_input_layout);
    }

    retval.set_depth_stencil_format(DXGI_FORMAT_D32_FLOAT)
        .set_depth_state(true)
//.set_two_sided(true)// TODO
        .set_primitive_topology(get_primitive_topology_type(shader_code))
        .set_render_targets(DXGI_FORMAT_R8G8B8A8_UNORM)
        .set_sample_desc();

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_pipeline_state
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::sphere_benchmark_base::get_pipeline_state(ID3D12Device *device,
        const shader_id_type shader_code) {
    assert(device != nullptr);
    auto it = this->_pipeline_cache.find(shader_code);
    auto is_create = (it == this->_pipeline_cache.end());

    if (!is_create) {
        // If the device was switched, we need to recreate the pipeline state
        // on the new device.
        auto existing_device = get_device(it->second);
        is_create = (existing_device != device);
    }

    if (is_create) {
        log::instance().write_line(log_level::debug, "Building pipeline state "
            "for shader code 0x{:x} ...", shader_code);
        auto builder = this->get_pipeline_builder(shader_code);
        auto retval = this->_pipeline_cache[shader_code] = builder.build(device);
        set_debug_object_name(retval, "Pipeline state \"0x{:x}\"", shader_code);
        return retval;

    } else {
        return it->second;
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_root_signature
 */
ATL::CComPtr<ID3D12RootSignature>
trrojan::d3d12::sphere_benchmark_base::get_root_signature(ID3D12Device *device,
        const shader_id_type shader_code) {
    assert(device != nullptr);
    auto it = this->_root_sig_cache.find(shader_code);
    auto is_create = (it == this->_root_sig_cache.end());

    if (!is_create) {
        // If the device was switched, we need to recreate the root signature
        // on the new device.
        auto existing_device = get_device(it->second);
        is_create = (existing_device != device);
    }

    if (is_create) {
        auto builder = this->get_pipeline_builder(shader_code);
        auto retval = this->_root_sig_cache[shader_code]
            = graphics_pipeline_builder::root_signature_from_shader(device,
                builder);
        set_debug_object_name(retval, "Root signature \"0x{:x}\"", shader_code);
        return retval;

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
 * trrojan::d3d12::sphere_benchmark_base::get_sphere_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_sphere_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);
    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_tessellation_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    const auto buffers = this->pipeline_depth();
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();

    // Skip all sphere and view constants.
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);
    retval = offset_by_n<ViewConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);

    retval = offset_by_n<TessellationConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_view_constants
 */
void trrojan::d3d12::sphere_benchmark_base::get_view_constants(
        ViewConstants& out_constants) const {
    auto projection = this->_camera.calc_projection_mxz0();
    out_constants.ProjMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(projection));

    auto& view = this->_camera.get_view_mx();
    out_constants.ViewMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(view));

    auto viewInv = glm::inverse(view);
    out_constants.ViewInvMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewInv));

    auto viewProj = projection * view;
    out_constants.ViewProjMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewProj));

    auto viewProjInv = glm::inverse(viewProj);
    out_constants.ViewProjInvMatrix[0] = DirectX::XMFLOAT4X4(
        glm::value_ptr(viewProjInv));

    const auto& viewport = this->viewport();
    out_constants.Viewport.x = viewport.TopLeftX;
    out_constants.Viewport.y = viewport.TopLeftY;
    out_constants.Viewport.z = viewport.Width;
    out_constants.Viewport.w = viewport.Height;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::get_view_constants
 */
D3D12_GPU_VIRTUAL_ADDRESS
trrojan::d3d12::sphere_benchmark_base::get_view_constants(
        const UINT buffer) const {
    assert(this->_constant_buffer != nullptr);
    const auto buffers = this->pipeline_depth();
    auto retval = this->_constant_buffer->GetGPUVirtualAddress();

    // Skip all SphereConstants.
    retval = offset_by_n<SphereConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffers);

    retval = offset_by_n<ViewConstants,
        D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(retval, buffer);

    return retval;
}


/*
 * trrojan::d3d12::sphere_benchmark_base::load_data
 */
ATL::CComPtr<ID3D12Resource> trrojan::d3d12::sphere_benchmark_base::load_data(
        ID3D12GraphicsCommandList *cmd_list, const shader_id_type shader_code,
        const configuration& config, const D3D12_RESOURCE_STATES state) {
    assert(cmd_list != nullptr);
    auto device = get_device(cmd_list);
    ATL::CComPtr<ID3D12Resource> retval;
    UINT64 size = 0;

    try {
        auto desc = parse_random_sphere_desc(config, shader_code);
        this->set_properties(desc);

        void *data;
        size = random_sphere_generator::create(nullptr, 0, desc);
        this->_data = create_buffer(device, size);
        retval = create_upload_buffer(this->_data);

        auto hr = retval->Map(0, nullptr, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        try {
            random_sphere_generator::create(data, size, this->_max_radius,
                desc);
            retval->Unmap(0, nullptr);
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

        mmpld::file<std::ifstream> file(path.c_str());
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

        size = mmpld::get_size<UINT64>(list_header);
        this->_data = create_buffer(device, size);
        retval = create_upload_buffer(this->_data);

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
        } catch (...) {
            log::instance().write_line(log_level::error, "Failed to read "
                "MMPLD particles from \"{}\". The headers could be read, i.e."
                "the input file might be corrupted.", path);
            retval->Unmap(0, nullptr);
            throw;
        }
    }
    // At this point, the data are in the upload buffer and the read-only buffer
    // has been prepared as copy target.
    assert(this->_data != nullptr);
    assert(retval != nullptr);
    assert(size > 0);
    set_debug_object_name(this->_data, config.get<std::string>(
        factor_data_set).c_str());

    cmd_list->CopyBufferRegion(this->_data, 0, retval, 0, size);
    transition_resource(cmd_list, this->_data, D3D12_RESOURCE_STATE_COPY_DEST,
        state);

    return retval;
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

        mmpld::file<std::ifstream> file(path.c_str());
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
void trrojan::d3d12::sphere_benchmark_base::on_device_switch(device& device) {
    assert(device.d3d_device() != nullptr);
    static constexpr auto CONSTANT_BUFFER_SIZE
        = align_constant_buffer_size(sizeof(SphereConstants))
        + align_constant_buffer_size(sizeof(ViewConstants))
        + align_constant_buffer_size(sizeof(TessellationConstants));
    benchmark_base::on_device_switch(device);

    // PSOs and root sigs are device-specific, so clear all cached ones.
    this->_pipeline_cache.clear();
    this->_root_sig_cache.clear();

    // Resources are device-specific, so delete and recreate them.
    {
        auto cmd_list = this->create_graphics_command_list();
        set_debug_object_name(cmd_list, "Viridis initialisation command list");
        this->_colour_map = create_viridis_colour_map(device, cmd_list);
    }


    this->_constant_buffer = create_constant_buffer(device.d3d_device(),
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
        p = offset_by_n<SphereConstants,
            D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(
            p, this->pipeline_depth());

        this->_view_constants = static_cast<ViewConstants *>(p);
        p = offset_by_n<ViewConstants,
            D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT>(
                p, this->pipeline_depth());

        this->_tessellation_constants= static_cast<TessellationConstants *>(p);
        assert(static_cast<void *>(this->_sphere_constants)
            < static_cast<void *>(this->_view_constants));
        assert(static_cast<void *>(this->_view_constants)
            < static_cast<void *>(this->_tessellation_constants));
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_descriptors
 */
trrojan::d3d12::sphere_benchmark_base::descriptor_table_type
trrojan::d3d12::sphere_benchmark_base::set_descriptors(
        ID3D12Device *device, const shader_id_type shader_code,
        const UINT frame) {
    assert(device != nullptr);
    auto heap = this->_descriptor_heaps[frame];
    assert(heap->GetDesc().Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto cpu_handle = heap->GetCPUDescriptorHandleForHeapStart();
    auto gpu_handle = heap->GetGPUDescriptorHandleForHeapStart();
    const auto increment = device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> tables;

    // Create VS resource descriptors.
    const auto pv_intensity = is_technique(shader_code,
        SPHERE_INPUT_PV_INTENSITY);
    const auto use_instancing = is_technique(shader_code,
        SPHERE_TECHNIQUE_USE_INSTANCING);
    if (pv_intensity || use_instancing) {
        tables.push_back(gpu_handle);

        if (pv_intensity) {
            log::instance().write_line(log_level::debug, "Rendering technique "
                "uses per-vertex-colouring. Setting transfer function ...");
            device->CreateShaderResourceView(this->_colour_map, nullptr,
                cpu_handle);
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;
        }

        if (use_instancing) {
            log::instance().write_line(log_level::debug, "Rendering technique "
                "uses instancing. Setting structured buffer view ...");
            this->create_buffer_resource_view(this->_data, 0,
                this->_cnt_spheres, this->_stride, cpu_handle);
            cpu_handle.ptr += increment;
            gpu_handle.ptr += increment;
        }
    }

    // Create PS resource descriptors.
    if (is_any_technique(shader_code, SPHERE_INPUT_PP_INTENSITY)) {
        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "per-pixel colouring. Setting transfer function as t0.");
        tables.push_back(gpu_handle);

        device->CreateShaderResourceView(this->_colour_map, nullptr,
            cpu_handle);

        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;
    }

    // Bind the constant buffers. There are always the same independent from the
    // rendering technique used.
    {
        log::instance().write_line(log_level::debug, "Setting constant "
            "buffers.");
        tables.push_back(gpu_handle);

        auto sphere_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;

        auto view_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;

        auto tess_constants = cpu_handle;
        cpu_handle.ptr += increment;
        gpu_handle.ptr += increment;
        assert(sphere_constants.ptr < view_constants.ptr);
        assert(view_constants.ptr < tess_constants.ptr);

        this->create_constant_buffer_view(device, frame, sphere_constants,
            view_constants, tess_constants);

        //cmd_list->SetGraphicsRootConstantBufferView(0,
        //    this->get_sphere_constants(frame));
        //cmd_list->SetGraphicsRootConstantBufferView(1,
        //    this->get_tessellation_constants(frame));
        //cmd_list->SetGraphicsRootConstantBufferView(2,
        //    this->get_view_constants(frame));
    }

    return std::make_pair(heap, tables);
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
        desc.type);
    this->_input_layout = random_sphere_generator::get_input_layout<
        D3D12_INPUT_ELEMENT_DESC>(desc.type);
    this->_intensity_range[0] = 0.0f;
    this->_intensity_range[1] = 1.0f;

    // Without the actual data, we can only set the requested maximum size, not
    // the actually realised maximum.
    this->_max_radius = desc.sphere_size[1];

    this->_stride = random_sphere_generator::get_stride(desc.type);
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

    this->_stride = mmpld::get_stride<UINT>(header);
}


/*
 * trrojan::d3d12::sphere_benchmark_base::set_vertex_buffer
 */
void trrojan::d3d12::sphere_benchmark_base::set_vertex_buffer(
        ID3D12GraphicsCommandList *cmd_list, const shader_id_type shader_code) {
    assert(cmd_list != nullptr);
    if (!is_technique(shader_code, SPHERE_TECHNIQUE_USE_SRV)) {
        D3D12_VERTEX_BUFFER_VIEW desc;
        desc.BufferLocation = this->_data->GetGPUVirtualAddress();
        desc.SizeInBytes = this->_data->GetDesc().Width;
        desc.StrideInBytes = this->_stride;

        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "vertex buffer. Setting 0x{0:x} ...", desc.BufferLocation);
        cmd_list->IASetVertexBuffers(0, 1, &desc);
    }
}


/*
 * trrojan::d3d12::sphere_benchmark_base::update_constants
 */
void trrojan::d3d12::sphere_benchmark_base::update_constants(
        const configuration& config, const UINT buffer) {
    assert(buffer < this->pipeline_depth());
    this->get_sphere_constants(*index_constant_buffer(
        this->_sphere_constants, buffer));
    this->get_tessellation_constants(*index_constant_buffer(
        this->_tessellation_constants, buffer), config);
    this->get_view_constants(*index_constant_buffer(
        this->_view_constants, buffer));
}
