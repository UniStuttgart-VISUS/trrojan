/// <copyright file="mmpld_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/mmpld_base.h"

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
trrojan::d3d11::mmpld_base::mmpld_base(void) {
    ::memset(&this->mmpld_header, 0, sizeof(this->mmpld_header));
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
