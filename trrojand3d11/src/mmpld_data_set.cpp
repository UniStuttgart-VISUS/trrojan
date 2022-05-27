/// <copyright file="mmpld_data_set.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/mmpld_data_set.h"

#include <glm/glm.hpp>

#include <cassert>
#include <cinttypes>
#include <limits>
#include <sstream>
#include <stdexcept>

#include "trrojan/log.h"

#include "sphere_techniques.h"


/*
 * trrojan::d3d11::mmpld_data_set::create
 */
trrojan::d3d11::sphere_data_set trrojan::d3d11::mmpld_data_set::create(
        const char *path) {
    std::shared_ptr<mmpld_data_set> retval(new mmpld_data_set());
    retval->open(path);
    return retval;
}


/*
 * trrojan::d3d11::mmpld_data_set::load_flag_fit_bounding_box
 */
const trrojan::d3d11::mmpld_data_set::frame_load_flags
trrojan::d3d11::mmpld_data_set::load_flag_fit_bounding_box
    = SPHERE_TECHNIQUE_RESERVED_MMPLD;


/*
 * trrojan::d3d11::mmpld_data_set::load_flag_float_colour
 */
const trrojan::d3d11::mmpld_data_set::frame_load_flags
trrojan::d3d11::mmpld_data_set::load_flag_float_colour
    = trrojan::d3d11::sphere_data_set_base::property_float_colour;



/*
 * trrojan::d3d11::mmpld_data_set::load_flag_structured_resource
 */
const trrojan::d3d11::mmpld_data_set::frame_load_flags
trrojan::d3d11::mmpld_data_set::load_flag_structured_resource
    = trrojan::d3d11::sphere_data_set_base::property_structured_resource;


/*
 * trrojan::d3d11::mmpld_data_set::get_mmpld_layout
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::mmpld_data_set::get_input_layout(
        const mmpld_reader::list_header& header) {
    D3D11_INPUT_ELEMENT_DESC element;
    UINT offset = 0;
    std::vector<D3D11_INPUT_ELEMENT_DESC> retval;

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "POSITION";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (header.vertex_type) {
        case mmpld_reader::vertex_type::float_xyz:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::vertex_type::float_xyzr:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case  mmpld_reader::vertex_type::short_xyz:
            throw std::runtime_error("vertex_type::short_xyz is not "
                "supported with Direct3D 11.");
    }

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "COLOR";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (header.colour_type) {
        case mmpld_reader::colour_type::float_i:
            element.Format = DXGI_FORMAT_R32_FLOAT;
            offset += 1 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::float_rgb:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::float_rgba:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::uint8_rgb:
            throw std::runtime_error("colour_type::uint8_rgb is not "
                "supported with Direct3D 11.");

        case mmpld_reader::colour_type::uint8_rgba:
            element.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;
    }

    return std::move(retval);
}


/*
 * trrojan::d3d11::mmpld_data_set::get_properties
 */
trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::mmpld_data_set::get_properties(
        const mmpld_reader::list_header& header) {
    properties_type retval = 0;

    switch (header.vertex_type) {
        case mmpld_reader::vertex_type::float_xyzr:
            retval |= property_per_sphere_radius;
            break;
    }

    switch (header.colour_type) {
        case mmpld_reader::colour_type::float_i:
            retval |= property_per_sphere_intensity;
            break;

        case mmpld_reader::colour_type::float_rgb:
        case mmpld_reader::colour_type::float_rgba:
            retval |= property_float_colour;
            /* falls through. */
        case mmpld_reader::colour_type::uint8_rgb:
        case mmpld_reader::colour_type::uint8_rgba:
            retval |= property_per_sphere_colour;
            break;
    }

    return retval;
}


/*
 * trrojan::d3d11::mmpld_data_set::bounding_box
 */
void trrojan::d3d11::mmpld_data_set::bounding_box(
        point_type& outMin, point_type& outMax) const {
    for (size_t i = 0; i < outMin.size(); ++i) {
        outMin[i] = this->_header.bounding_box[i];
    }
    for (size_t i = 0; i < outMax.size(); ++i) {
        outMax[i] = this->_header.bounding_box[i + 3];
    }
}


/*
 * trrojan::d3d11::mmpld_data_set::max_radius
 */
float trrojan::d3d11::mmpld_data_set::max_radius(void) const {
    return this->_max_radius;
}



/*
 * trrojan::d3d11::mmpld_data_set::read_frame
 */
trrojan::d3d11::rendering_technique::buffer_type
trrojan::d3d11::mmpld_data_set::read_frame(ID3D11Device *device,
        const unsigned int frame, const frame_load_flags options) {
    assert(this->_stream.good());
    assert(device != nullptr);
    static const frame_load_flags VALID_INPUT_FLAGS // Flags directly copied from user input.
        = sphere_data_set_base::property_structured_resource;

    D3D11_BUFFER_DESC bufferDesc;
    std::vector<char> data;
    mmpld_reader::frame_header frameHeader;
    D3D11_SUBRESOURCE_DATA id;
    rendering_technique::buffer_type retval;

    // Make sure to erase last layout in case I/O fails.
    this->_layout.clear();
    ::memset(&this->_list, 0, sizeof(this->_list));

    // Basic sanity check.
    if (frame >= this->_seek_table.size()) {
        std::stringstream msg;
        msg << "The requested frame #" << frame << " does not exists. The file "
            << "comprises only " << this->_seek_table.size()
            << " frame(s)." << std::ends;
        throw std::invalid_argument(msg.str());
    }

    // Read the list header and determine the layout using the header.
    this->_stream.seekg(this->_seek_table[frame]);
    mmpld_reader::read_frame_header(frameHeader, this->_stream,
        this->_header.version);

    if (frameHeader.lists > 1) {
        log::instance().write_line(log_level::warning, "TRRojan only supports "
            "MMPLD files with one particle list per frame. All but the first "
            "will be ignored.");
    }

    // Read the list header and prepare the D3D input layout.
    mmpld_reader::read_list_header(this->_list, this->_stream);
    this->_layout = mmpld_data_set::get_input_layout(this->_list);

    // Read the data.
    auto cntData = this->stride() * this->size();
    data.resize(cntData);
    this->_stream.read(data.data(), cntData);

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

    // Search the maximum radius for clipping.
    if (this->_list.radius > 0.0f) {
        this->_max_radius = this->_list.radius;

    } else {
        this->_max_radius = (std::numeric_limits<float>::min)();
        auto stride = this->stride();

        for (auto cur = data.data(), end = data.data() + data.size();
                cur < end; cur += stride) {
            auto r = *reinterpret_cast<float *>(cur + 3 * sizeof(float));
            if (r > this->_max_radius) {
                this->_max_radius = r;
            }
        }
    }

    // Recompute bounding box as requested.
    if ((options & load_flag_fit_bounding_box) != 0) {
        log::instance().write_line(log_level::verbose, "Recomputing bounding "
            "box of MMPLD data from data actually contained in the active "
            "particle list ...");

        typedef std::decay<decltype(*this->_header.bounding_box)>::type bbox_type;
        typedef std::numeric_limits<bbox_type> bbox_limits;
        const auto stride = this->stride();

        // Re-initialise with extrema.
        for (size_t c = 0; c < 3; ++c) {
            this->_header.bounding_box[c] = (bbox_limits::max)();
            this->_header.bounding_box[c + 3] = (bbox_limits::lowest)();
        }

        // Search minimum and maximum.
        for (size_t i = 0; i < this->_list.particles; ++i) {
            auto pos = reinterpret_cast<const float *>(
                data.data() + i * stride);
            for (size_t c = 0; c < 3; ++c) {
                if (pos[c] < this->_header.bounding_box[c]) {
                    this->_header.bounding_box[c] = pos[c];
                }
                if (pos[c] > this->_header.bounding_box[c + 3]) {
                    this->_header.bounding_box[c + 3] = pos[c];
                }
            }
        }

        // Account for the radius.
        for (size_t c = 0; c < 3; ++c) {
            this->_header.bounding_box[c] -= this->_max_radius;
            this->_header.bounding_box[c + 3] += this->_max_radius;
        }

        log::instance().write_line(log_level::verbose, "Recomputed "
            "single-frame bounding box of MMPLD data is ({}, {}, {}) - "
            "({}, {}, {}) with maximum radius of {}.",
            this->_header.bounding_box[0], this->_header.bounding_box[1],
            this->_header.bounding_box[2], this->_header.bounding_box[3],
            this->_header.bounding_box[4], this->_header.bounding_box[5],
            this->_max_radius);
    } /* end if ((options & load_flag_fit_bounding_box) != 0) */

    // If everything succeeded, create the vertex buffer.
    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    assert(cntData <= UINT_MAX);
    bufferDesc.ByteWidth = static_cast<UINT>(cntData);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.CPUAccessFlags = 0;
    if ((options & property_structured_resource) != 0) {
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.StructureByteStride = this->stride();
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    } else {
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    }

    ::ZeroMemory(&id, sizeof(id));
    id.pSysMem = data.data();

    auto hr = device->CreateBuffer(&bufferDesc, &id, &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create vertex buffer from MMPLD with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    set_debug_object_name(retval.p, "mmpld_data_set");

    this->_buffer = retval;
    this->_properties = (options & VALID_INPUT_FLAGS);
    this->_properties |= mmpld_data_set::get_properties(this->_list);

    return retval;
}


/*
 * trrojan::d3d11::mmpld_data_set::size
 */
trrojan::d3d11::mmpld_data_set::size_type
trrojan::d3d11::mmpld_data_set::size(void) const {
    assert(this->_list.particles < (std::numeric_limits<size_type>::max)());
    return (this->_stream.good())
        ? static_cast<size_type>(this->_list.particles)
        : static_cast<size_type>(0);
}


/*
 * trrojan::d3d11::mmpld_data_set::stride
 */
trrojan::d3d11::mmpld_data_set::size_type
trrojan::d3d11::mmpld_data_set::stride(void) const {
    auto retval = mmpld_reader::calc_stride(this->_list);
    assert(retval < (std::numeric_limits<size_type>::max)());
    return static_cast<size_type>(retval);
}


/*
 * trrojan::d3d11::mmpld_data_set::is_non_float_colour
 */
bool trrojan::d3d11::mmpld_data_set::is_non_float_colour(
        const mmpld_reader::list_header& list) {
    switch (list.colour_type) {
        case mmpld_reader::colour_type::uint8_rgb:
        case mmpld_reader::colour_type::uint8_rgba:
            return true;

        case mmpld_reader::colour_type::none:
            // Note: no colour is also not a non-float colour.
        default:
            return false;
    }
}


/*
 * trrojan::d3d11::mmpld_data_set::mmpld_data_set
 */
trrojan::d3d11::mmpld_data_set::mmpld_data_set(void) : _max_radius(0.0f) {
    ::memset(&this->_header, 0, sizeof(this->_header));
    ::memset(&this->_list, 0, sizeof(this->_list));
}


/*
 * trrojan::d3d11::mmpld_data_set::get_mmpld_input_properties
 */
trrojan::d3d11::mmpld_data_set::mmpld_input_properties
trrojan::d3d11::mmpld_data_set::get_mmpld_input_properties(void) const  {
    typedef mmpld_reader::shader_properties sp_t;
    auto retval = static_cast<mmpld_input_properties>(
        mmpld_reader::calc_shader_properties(this->_list));
    log::instance().write_line(log_level::debug, "Computed MMPLD input "
        "properties as {}.", retval);
    return retval;
}


/*
 * trrojan::d3d11::mmpld_data_set::open
 */
bool trrojan::d3d11::mmpld_data_set::open(const char *path) {
    mmpld_reader::read_file_header(this->_stream, this->_header,
        this->_seek_table, path);
    return this->_stream.good();
}

