// <copyright file="sphere_data.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_data.h"

#include <fstream>

#include "trrojan/log.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/sphere_rendering_configuration.h"

#include "sphere_techniques.h"
#include "SpherePipeline.hlsli"


/*
 * trrojan::d3d12::sphere_data::is_non_float_colour
 */
bool trrojan::d3d12::sphere_data::is_non_float_colour(
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
 * trrojan::d3d12::sphere_data::parse_random_sphere_desc
 */
trrojan::random_sphere_generator::description
trrojan::d3d12::sphere_data::parse_random_sphere_desc(
        const sphere_rendering_configuration& config,
        const shader_id_type shader_code) {
    static const auto ERASE_PROPERTIES = ~static_cast<property_mask_type>(
        property_structured_resource);

    auto flags = static_cast<random_sphere_generator::create_flags>(
        shader_code & ERASE_PROPERTIES);
    if (config.force_float_colour()) {
        flags |= random_sphere_generator::create_flags::float_colour;
    }

    return random_sphere_generator::parse_description(config.data_set(), flags);
}


/*
 * trrojan::d3d12::sphere_data::sphere_data
 */
trrojan::d3d12::sphere_data::sphere_data(void)
    : _bbox({ glm::vec3(0.0f), glm::vec3(0.0f) }),
    _cnt_spheres(0),
    _colour({ 0.0f, 0.0f, 0.0f, 0.0f }),
    _intensity_range({ 0.0f, 0.0f }),
    _max_radius(0.0f),
    _properties(properties_type::none),
    _stride(0) { }


/*
 * trrojan::d3d12::sphere_data::adjust_shader_code
 */
trrojan::d3d12::sphere_data::shader_id_type
trrojan::d3d12::sphere_data::adjust_shader_code(
        shader_id_type shader_code) const {
    // First, get only the data properties relevant for the shader code.
    auto data_code = this->properties(shader_code);

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
 * trrojan::d3d12::sphere_data::clear
 */
void trrojan::d3d12::sphere_data::clear(void) {
    this->_bbox = { glm::vec3(0.0f), glm::vec3(0.0f) };
    this->_cnt_spheres = 0;
    this->_colour = { 0.0f, 0.0f, 0.0f, 0.0f };
    this->_data = nullptr;
    this->_intensity_range = { 0.0f, 0.0f };
    this->_max_radius = 0.0f;
    this->_properties = properties_type::none;
    this->_stride = 0;
}


/*
 * trrojan::d3d12::sphere_data::copy_to
 */
winrt::file_handle trrojan::d3d12::sphere_data::copy_to(const std::string& path,
        const shader_id_type shader_code,
        const sphere_rendering_configuration& config) {
    winrt::file_handle retval(::CreateFileA(path.c_str(),
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL));
    if (!retval) {
        throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
    }

    this->load([&](const UINT64 size) {
        //this->_buffer.resize(size);
        //return this->_buffer.data();
        return nullptr;
    }, shader_code, config);


    return retval;
}


/*
 * trrojan::d3d12::sphere_data::extents
 */
std::array<float, 3> trrojan::d3d12::sphere_data::extents(void) const {
    std::array<float, 3> retval = {
        std::abs(this->_bbox[1].x - this->_bbox[0].x),
        std::abs(this->_bbox[1].y - this->_bbox[0].y),
        std::abs(this->_bbox[1].z - this->_bbox[0].z)
    };
    return retval;
}


/*
 * trrojan::d3d12::sphere_data::get_sphere_constants
 */
void trrojan::d3d12::sphere_data::get_sphere_constants(
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
 * trrojan::d3d12::sphere_data::load
 */
ATL::CComPtr<ID3D12Resource> trrojan::d3d12::sphere_data::load(
        ID3D12GraphicsCommandList *cmd_list,
        const shader_id_type shader_code,
        const sphere_rendering_configuration& config,
        const D3D12_RESOURCE_STATES state) {
    assert(cmd_list != nullptr);
    auto device = get_device(cmd_list);
    ATL::CComPtr<ID3D12Resource> retval;

    auto size = this->load([this, &device, &retval](const UINT64 size) {
        this->_data = create_buffer(device, size);
        retval = create_upload_buffer_for(this->_data);

        void *data;
        auto hr = retval->Map(0, nullptr, &data);
        if (FAILED(hr)) {
            // Note that we do not catch this outside, because if this throws,
            // load_data will exit with an exception and 'retval' will be
            // destroyed and thus unmapped implicitly.
            throw ATL::CAtlException(hr);
        }

        return data;
    }, shader_code, config);
    // At this point, the data are in the upload buffer and the read-only buffer
    // has been prepared as copy target.

    assert(retval != nullptr);
    retval->Unmap(0, nullptr);

    assert(this->_data != nullptr);
    set_debug_object_name(this->_data, config.data_set().c_str());

    cmd_list->CopyBufferRegion(this->_data, 0, retval, 0, size);
    transition_resource(cmd_list, this->_data, D3D12_RESOURCE_STATE_COPY_DEST,
        state);

    return retval;
}


/*
 * trrojan::d3d12::sphere_data::load
 */
UINT64 trrojan::d3d12::sphere_data::load(
        std::function<void *(const UINT64)> allocator,
        const shader_id_type shader_code,
        const sphere_rendering_configuration& config) {
    UINT64 retval;

    try {
        // First, try to parse the data set as specification of random spheres.
        // This will throw if the string is not a valid specification, in which
        // case we will try to open it from the file system.
        auto desc = parse_random_sphere_desc(config, shader_code);
        this->set_properties(desc);

        log::instance().write_line(log_level::information, "Create random "
            "sphere data set \"{}\".", config.data_set());
        retval = random_sphere_generator::create(nullptr, 0, desc);
        auto data = allocator(retval);

        try {
            random_sphere_generator::create(data, retval, this->_max_radius,
                desc);
        } catch (...) {
            log::instance().write_line(log_level::error, "Failed to create "
                "random sphere data for specification \"{}\". The "
                "specification was correct, but the generation threw an "
                "exception.", config.data_set());
            throw;
        }

    } catch (...) {
        // If it is not a random sphere specification, interpret it as the path
        // to an MMPLD file.
        mmpld::list_header list_header;
        const auto fit_bbox = config.fit_bounding_box();
        const auto force_float = config.force_float_colour();
        const auto path = config.data_set();
        const auto frame = config.frame();

        log::instance().write_line(log_level::information, "Load MMPLD data "
            "set \"{}\".", path);
#if defined(TRROJAN_FOR_UWP)
        mmpld::file<std::ifstream> file(path.c_str());
#else /* defined(TRROJAN_FOR_UWP) */
        mmpld::file<HANDLE> file(path.c_str());
#endif /* defined(TRROJAN_FOR_UWP) */
        file.open_frame(frame);
        file.read_particles(false, list_header, nullptr, 0);
        this->set_properties(file.file_header(), list_header);

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

        retval = mmpld::get_size<UINT64>(list_header);
        auto data = allocator(retval);

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
                file.read_particles(list_header, data, retval);
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
        } catch (...) {
            log::instance().write_line(log_level::error, "Failed to read "
                "MMPLD particles from \"{}\". The headers could be read, i.e."
                "the input file might be corrupted.", path);
            throw;
        }
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_data::load_properties
 */
void trrojan::d3d12::sphere_data::load_properties(
        const shader_id_type shader_code,
        const sphere_rendering_configuration& config) {
    try {
        auto desc = parse_random_sphere_desc(config, shader_code);
        log::instance().write_line(log_level::verbose, "Retrieving properties "
            "of random spheres \"{}\" ...", config.data_set());
        this->set_properties(desc);

    } catch (...) {
        const auto force_float = config.force_float_colour();
        const auto frame = config.frame();
        auto path = config.data_set();
        log::instance().write_line(log_level::verbose, "Retrieving properties "
            "of frame {0} in MMPLD data set \"{}\" ...", frame, path);
        mmpld::list_header list_header;

#if defined(TRROJAN_FOR_UWP)
        mmpld::file<std::ifstream> file(path.c_str());
#else /* defined(TRROJAN_FOR_UWP) */
        mmpld::file<HANDLE> file(path.c_str());
#endif /* defined(TRROJAN_FOR_UWP) */
        file.open_frame(frame);
        file.read_particles(list_header, nullptr, 0);

        if (force_float && is_non_float_colour(list_header)) {
            list_header.colour_type = mmpld::colour_type::rgba32;
        }

        this->set_properties(file.file_header(), list_header);
    }
}


/*
 * trrojan::d3d12::sphere_data::properties
 */
trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::properties(
        const shader_id_type shader_code) const {
    static const shader_id_type LET_TECHNIQUE_DECIDE
        = (SPHERE_INPUT_PV_INTENSITY | SPHERE_INPUT_PP_INTENSITY);

    auto retval = static_cast<property_mask_type>(this->_properties);

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
 * trrojan::d3d12::sphere_data::property_float_colour
 */
const trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::property_float_colour
    = SPHERE_INPUT_FLT_COLOUR;
static_assert(mmpld::particle_properties::float_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_FLT_COLOUR),
    "Constant value SPHERE_INPUT_FLT_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_data::property_per_sphere_colour
 */
const trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::property_per_sphere_colour
    = SPHERE_INPUT_PV_COLOUR;
static_assert(mmpld::particle_properties::per_particle_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_COLOUR),
    "Constant value SPHERE_INPUT_PV_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_data::property_per_sphere_intensity
 */
const trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::property_per_sphere_intensity
    = SPHERE_INPUT_PP_INTENSITY | SPHERE_INPUT_PV_INTENSITY;
static_assert(mmpld::particle_properties::per_particle_intensity
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_INTENSITY),
    "Constant value SPHERE_INPUT_PV_INTENSITY must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_data::property_per_sphere_radius
 */
const trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::property_per_sphere_radius
    = SPHERE_INPUT_PV_RADIUS;
static_assert(mmpld::particle_properties::per_particle_radius
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_RADIUS),
    "Constant value SPHERE_INPUT_PV_RADIUS must match MMLPD library.");


/*
 * trrojan::d3d12::sphere_data::property_structured_resource
 */
const trrojan::d3d12::sphere_data::property_mask_type
trrojan::d3d12::sphere_data::property_structured_resource
    = SPHERE_TECHNIQUE_USE_SRV;


/*
 * trrojan::d3d12::sphere_data::fit_bounding_box
 */
void trrojan::d3d12::sphere_data::fit_bounding_box(
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
 * trrojan::d3d12::sphere_data::set_max_radius
 */
void trrojan::d3d12::sphere_data::set_max_radius(
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
            if (pos[3] > this->_max_radius) {
                this->_max_radius = pos[3];
            }
        }
    }

    log::instance().write_line(log_level::verbose, "Maximum radius in MMPLD "
        "particle list was computed as {}.", this->_max_radius);
}


/*
 * trrojan::d3d12::sphere_data::set_properties
 */
void trrojan::d3d12::sphere_data::set_properties(
        const random_sphere_generator::description& desc) {
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        this->_bbox[0][i] = -0.5f * desc.domain_size[i];
        this->_bbox[1][i] = 0.5f * desc.domain_size[i];
    }

    this->_cnt_spheres = static_cast<UINT>(desc.number);
    this->_colour[0] = 0.5f;
    this->_colour[1] = 0.5f;
    this->_colour[2] = 0.5f;
    this->_colour[3] = 1.0f;
    this->_properties = random_sphere_generator::get_properties(
        desc.type);
    this->_input_layout = random_sphere_generator::get_input_layout<
        D3D12_INPUT_ELEMENT_DESC>(desc.type);
    this->_intensity_range[0] = 0.0f;
    this->_intensity_range[1] = 1.0f;

    // Without the actual data, we can only set the requested maximum size, not
    // the actually realised maximum.
    this->_max_radius = desc.sphere_size[1];

    this->_stride = static_cast<UINT>(random_sphere_generator::get_stride(
        desc.type));
}


/*
 * trrojan::d3d12::sphere_data::set_properties
 */
void trrojan::d3d12::sphere_data::set_properties(
        const mmpld::file_header& file_header,
        const mmpld::list_header& list_header) {
    for (glm::length_t i = 0; i < this->_bbox[0].length(); ++i) {
        // TODO: list_header can theoretically override bbox.
        this->_bbox[0][i] = file_header.bounding_box[i];
        this->_bbox[1][i] = file_header.bounding_box[i + 3];
    }

    this->_cnt_spheres = static_cast<UINT>(list_header.particles);
    ::memcpy(this->_colour.data(),
        &list_header.colour,
        sizeof(list_header.colour));
    this->_properties = mmpld::get_properties<properties_type>(list_header);
    this->_input_layout = mmpld::get_input_layout<D3D12_INPUT_ELEMENT_DESC>(
        list_header);
    this->_intensity_range[0] = list_header.min_intensity;
    this->_intensity_range[1] = list_header.max_intensity;

    // The global radius from the list header might be wrong, but we cannot do
    // any better than this without the actual data.
    this->_max_radius = list_header.radius;

    this->_stride = mmpld::get_stride<UINT>(list_header);
}
