/// <copyright file="mmpld_reader.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/mmpld_reader.h"

#include <climits>
#include <cstring>
#include <fstream>
#include <stdexcept>


/*
 * trrojan::mmpld_reader::calc_stride
 */
std::uint64_t trrojan::mmpld_reader::calc_stride(const list_header& header) {
    std::uint64_t retval = 0;

    switch (header.vertex_type) {
        case vertex_type::float_xyz:
            retval += 12;
            break;

        case vertex_type::float_xyzr:
            retval += 16;
            break;

        case vertex_type::short_xyz:
            retval += 6;
            break;
    }

    switch (header.colour_type) {
        case colour_type::uint8_rgb:
            retval += 3;
            break;

        case colour_type::uint8_rgba:
            retval += 4;
            break;

        case colour_type::float_i:
            retval += 4;
            break;

        case colour_type::float_rgb:
            retval += 12;
            break;

        case colour_type::float_rgba:
            retval += 16;
            break;
    }

    return retval;
}


///*
// * mmpld_reader::ParseListHeader
// */
//std::vector<D3D11_INPUT_ELEMENT_DESC> mmpld_reader::ParseListHeader(
//        const ListHeader& header) {
//    D3D11_INPUT_ELEMENT_DESC element;
//    UINT offset = 0;
//    std::vector<D3D11_INPUT_ELEMENT_DESC> retval;
//
//    the::zero_memory(&element);
//    element.SemanticName = "POSITION";
//    element.AlignedByteOffset = offset;
//    element.InputSlotClass
//        = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
//
//    switch (header.Types.vertex_type) {
//        case vertex_type::FLOAT_XYZ:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
//            offset += 3 * sizeof(float);
//            retval.push_back(element);
//            break;
//
//        case vertex_type::FLOAT_XYZR:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
//            offset += 4 * sizeof(float);
//            retval.push_back(element);
//            break;
//
//        case vertex_type::SHORT_XYZ:
//            THROW_THE_EXCEPTION(the::argument_exception, _T("SHORT_XYZ is")
//                _T("unsupported."));
//            break;
//    }
//
//    the::zero_memory(&element);
//    element.SemanticName = "COLOR";
//    element.AlignedByteOffset = offset;
//    element.InputSlotClass
//        = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
//
//    switch (header.Types.colour_type) {
//        case colour_type::FLOAT_I:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
//            offset += 1 * sizeof(float);
//            retval.push_back(element);
//            break;
//
//        case colour_type::FLOAT_RGB:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT;
//            offset += 3 * sizeof(float);
//            retval.push_back(element);
//            break;
//
//        case colour_type::FLOAT_RGBA:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
//            offset += 4 * sizeof(float);
//            retval.push_back(element);
//            break;
//
//        case colour_type::UINT8_RGB:
//            THROW_THE_EXCEPTION(the::argument_exception, _T("UINT8_RGB is")
//                _T("unsupported."));
//            break;
//
//        case colour_type::UINT8_RGBA:
//            element.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
//            offset += 4 * sizeof(float);
//            retval.push_back(element);
//            break;
//    }
//
//    return std::move(retval);
//}


/*
 * trrojan::mmpld_reader::parse_version
 */
void trrojan::mmpld_reader::parse_version(int& outMajor, int& outMinor,
        const std::uint16_t version) {
    std::div_t v = std::div(version, 100);
    outMajor = v.quot;
    outMinor = v.rem;
}


/*
 * trrojan::mmpld_reader::read_file_header
 */
std::ifstream trrojan::mmpld_reader::read_file_header(file_header& outHeader,
        seek_table& outSeekTable, const char *path) {
    std::uint64_t offset = 0;

    if (path == nullptr) {
        throw std::invalid_argument("The path to the MMPLD file must not be a "
            "null pointer.");
    }

    // Open the file.
    std::ifstream retval(path, std::ios::binary);

    // Read and check the header.
    retval.read(reinterpret_cast<char *>(&outHeader), sizeof(outHeader));
    if (::strcmp(outHeader.magic_identifier, "MMPLD") != 0) {
        throw std::runtime_error("The given stream does not start with a valid "
            "MMPLD header.");
    }

    // Read the seek table.
    outSeekTable.clear();
    outSeekTable.reserve(outHeader.frames);
    for (uint32_t i = 0; i < outHeader.frames; ++i) {
        retval.read(reinterpret_cast<char *>(&offset), sizeof(offset));
        outSeekTable.push_back(offset);
    }

    return retval;
}


/*
 * trrojan::mmpld_reader::read_list_header
 */
void trrojan::mmpld_reader::read_list_header(list_header& outHeader,
        std::ifstream& file) {
    file.read(reinterpret_cast<char *>(&outHeader.vertex_type),
        sizeof(outHeader.vertex_type));
    file.read(reinterpret_cast<char *>(&outHeader.colour_type),
        sizeof(outHeader.colour_type));

    switch (outHeader.vertex_type) {
        case vertex_type::float_xyz:
        case vertex_type::short_xyz:
            file.read(reinterpret_cast<char *>(&outHeader.radius),
                sizeof(outHeader.radius));
            break;

        default:
            outHeader.radius = -1.0f;
            break;
    }

    switch (outHeader.colour_type) {
        case colour_type::none: {
            std::uint8_t rgba[4];
            file.read(reinterpret_cast<char *>(rgba), sizeof(rgba));
            for (size_t i = 0; i < 4; ++i) {
                outHeader.colour[i] = static_cast<float>(rgba[i])
                    / static_cast<float>(UCHAR_MAX);
            }
            outHeader.min_intensity = 0.0f;
            outHeader.max_intensity = -1.0f;
        } break;

        case colour_type::float_i:
            ::memset(&outHeader.colour, 0, sizeof(outHeader.colour));
            file.read(reinterpret_cast<char *>(&outHeader.min_intensity),
                sizeof(outHeader.min_intensity));
            file.read(reinterpret_cast<char *>(&outHeader.max_intensity),
                sizeof(outHeader.max_intensity));
            break;

        default:
            ::memset(&outHeader.colour, 0, sizeof(outHeader.colour));
            outHeader.min_intensity = 0.0f;
            outHeader.max_intensity = -1.0f;
            break;
    }

    file.read(reinterpret_cast<char *>(&outHeader.particles),
        sizeof(outHeader.particles));
}
