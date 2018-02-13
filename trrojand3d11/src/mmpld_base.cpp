/// <copyright file="mmpld_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/mmpld_base.h"

#include <glm/glm.hpp>

#include <cassert>
#include <cinttypes>
#include <sstream>
#include <stdexcept>

#include "trrojan/log.h"


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_layout
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::mmpld_base::get_mmpld_layout(
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
 * trrojan::d3d11::mmpld_base::mmpld_base
 */
trrojan::d3d11::mmpld_base::mmpld_base(void) : mmpld_max_radius(0) {
    ::memset(&this->mmpld_header, 0, sizeof(this->mmpld_header));
    ::memset(&this->mmpld_list, 0, sizeof(this->mmpld_list));
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_bounding_box
 */
void trrojan::d3d11::mmpld_base::get_mmpld_bounding_box(
        graphics_benchmark_base::point_type& outMin,
        graphics_benchmark_base::point_type& outMax) const {
    for (size_t i = 0; i < outMin.size(); ++i) {
        outMin[i] = this->mmpld_header.bounding_box[i];
    }
    for (size_t i = 0; i < outMax.size(); ++i) {
        outMax[i] = this->mmpld_header.bounding_box[i + 3];
    }
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_centre
 */
trrojan::graphics_benchmark_base::point_type
trrojan::d3d11::mmpld_base::get_mmpld_centre(void) const {
    graphics_benchmark_base::point_type retval = this->get_mmpld_size();

    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] /= 2.0f;
        retval[i] += this->mmpld_header.bounding_box[i];
    }

    return retval;
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_clipping
 */
std::pair<float, float> trrojan::d3d11::mmpld_base::get_mmpld_clipping(
        const camera& cam) const {
    auto& camPos = cam.get_look_from();
    glm::vec3 centre;
    auto diagLen = 0.0f;
    auto size = this->get_mmpld_size();

    for (size_t i = 0; i < size.size(); ++i) {
        centre[i] = size[i] / 2.0f + this->mmpld_header.bounding_box[i];
        diagLen += size[i] * size[i];
    }
    diagLen = std::sqrt(diagLen);

    diagLen += 0.5f * this->mmpld_max_radius;
    diagLen *= 0.5f;

    auto viewLen = glm::length(centre - camPos);

    //auto nearPlane = viewLen - diagLen;
    //if (nearPlane < 0.01f) {
    //    nearPlane = 0.01f;
    //}
    auto nearPlane = 0.01f;

    auto farPlane = viewLen + diagLen;
    if (farPlane < nearPlane) {
        farPlane = nearPlane + 1.0f;
    }

    return std::make_pair(nearPlane, farPlane);
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_pixel_shader_properties
 */
trrojan::d3d11::mmpld_base::shader_properties
trrojan::d3d11::mmpld_base::get_mmpld_pixel_shader_properties(
        const bool vsXfer) const {
    typedef mmpld_reader::shader_properties sp_t;

    shader_properties retval = mmpld_reader::calc_shader_properties(
        this->mmpld_list);

    if (vsXfer) {
        // If per-vertex transfer function was selected, erase the flag from
        // the pixel shader's properties.
        retval &= ~sp_t::per_vertex_intensity;
    }

    // The following per-vertex properties are irrelevant for the pixel shader.
    retval &= ~sp_t::per_vertex_colour;
    retval &= ~sp_t::per_vertex_radius;

    log::instance().write_line(log_level::debug, "Computed sphere pixel shader "
        "properties as %u.", retval);
    return static_cast<shader_properties>(retval);
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_size
 */
std::array<float, 3> trrojan::d3d11::mmpld_base::get_mmpld_size(void) const {
    std::array<float, 3> retval;

    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] = std::abs(this->mmpld_header.bounding_box[i + 3]
            - this->mmpld_header.bounding_box[i]);
    }

    return retval;
}


/*
 * trrojan::d3d11::mmpld_base::get_mmpld_vertex_shader_properties
 */
trrojan::d3d11::mmpld_base::shader_properties
trrojan::d3d11::mmpld_base::get_mmpld_vertex_shader_properties(
        const bool vsXfer) const  {
    typedef mmpld_reader::shader_properties sp_t;

    shader_properties retval = mmpld_reader::calc_shader_properties(
        this->mmpld_list);

    if (!vsXfer) {
        // If per-pixel transfer function was selected, erase the flag from
        // the pixel shader's properties.
        retval &= ~sp_t::intensity_xfer_function;
    }

    log::instance().write_line(log_level::debug, "Computed sphere vertex shader "
        "properties as %u.", retval);
    return static_cast<shader_properties>(retval);
}


/*
 * trrojan::d3d11::mmpld_base::open_mmpld
 */
bool trrojan::d3d11::mmpld_base::open_mmpld(const char *path) {
    mmpld_reader::read_file_header(this->mmpld_stream, this->mmpld_header,
        this->mmpld_seek_table, path);
    return this->mmpld_stream.good();
}


/*
 * trrojan::d3d11::mmpld_base::read_mmpld_frame
 */
ATL::CComPtr<ID3D11Buffer> trrojan::d3d11::mmpld_base::read_mmpld_frame(
        ID3D11Device *device, const unsigned int frame) {
    assert(this->mmpld_stream.good());
    assert(device != nullptr);

    D3D11_BUFFER_DESC bufferDesc;
    std::vector<char> data;
    mmpld_reader::frame_header frameHeader;
    D3D11_SUBRESOURCE_DATA id;
    ATL::CComPtr<ID3D11Buffer> retval;

    // Make sure to erase last layout in case I/O fails.
    this->mmpld_layout.clear();
    ::memset(&this->mmpld_list, 0, sizeof(this->mmpld_list));

    // Basic sanity check.
    if (frame >= this->mmpld_seek_table.size()) {
        std::stringstream msg;
        msg << "The requested frame #" << frame << " does not exists. The file "
            << "comprises only " << this->mmpld_seek_table.size()
            << " frame(s)." << std::ends;
        throw std::invalid_argument(msg.str());
    }

    // Read the list header and determine the layout using the header.
    this->mmpld_stream.seekg(this->mmpld_seek_table[frame]);
    mmpld_reader::read_frame_header(frameHeader, this->mmpld_stream,
        this->mmpld_header.version);

    if (frameHeader.lists > 1) {
        log::instance().write_line(log_level::warning, "TRRojan only supports "
            "MMPLD files with one particle list per frame. All but the first "
            "will be ignored.");
    }

    mmpld_reader::read_list_header(this->mmpld_list, this->mmpld_stream);
    this->mmpld_layout = mmpld_base::get_mmpld_layout(this->mmpld_list);

    // Read the data.
    auto cntData = mmpld_reader::calc_stride(this->mmpld_list) 
        * this->mmpld_list.particles;
    data.resize(cntData);
    this->mmpld_stream.read(data.data(), cntData);

    // Search the maximum radius for clipping.
    if (this->mmpld_list.radius > 0.0f) {
        this->mmpld_max_radius = this->mmpld_list.radius;

    } else {
        this->mmpld_max_radius = std::numeric_limits<float>::min();
        auto stride = mmpld_reader::calc_stride(this->mmpld_list);

        for (auto cur = data.data(), end = data.data() + data.size();
                cur < end; cur += stride) {
            auto r = *reinterpret_cast<float *>(cur + 3 * sizeof(float));
            if (r > this->mmpld_max_radius) {
                this->mmpld_max_radius = r;
            }
        }
    }

    // If everything succeeded, create the vertex buffer.
    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    assert(cntData <= UINT_MAX);
    bufferDesc.ByteWidth = static_cast<UINT>(cntData);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    ::ZeroMemory(&id, sizeof(id));
    id.pSysMem = data.data();

    auto hr = device->CreateBuffer(&bufferDesc, &id, &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create vertex buffer from MMPLD with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
