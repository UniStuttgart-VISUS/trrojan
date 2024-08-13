/// <copyright file="mmpld_reader.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/mmpld_reader.h"

#include <cinttypes>
#include <climits>
#include <cstring>
#include <fstream>
#include <stdexcept>

#include "trrojan/log.h"


/*
 * trrojan::mmpld_reader::shader_properties
 */
trrojan::mmpld_reader::shader_properties
trrojan::mmpld_reader::calc_shader_properties(const list_header& header) {
    std::underlying_type<shader_properties>::type retval = 0;

    switch (header.vertex_type) {
        case mmpld_reader::vertex_type::float_xyzr:
            retval |= shader_properties::per_vertex_radius;
            break;

        default:
            // Nothing to do.
            break;
    }

    switch (header.colour_type) {
        case mmpld_reader::colour_type::float_i:
            retval |= shader_properties::per_vertex_intensity;
            break;

        case mmpld_reader::colour_type::none:
            // Nothing to do.
            break;

        default:
            retval |= shader_properties::per_vertex_colour;
            break;
    }

    return static_cast<shader_properties>(retval);
}


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
std::ifstream& trrojan::mmpld_reader::read_file_header(std::ifstream& outStream,
        file_header& outHeader, seek_table& outSeekTable, const char *path) {
    std::uint64_t offset = 0;

    if (path == nullptr) {
        throw std::invalid_argument("The path to the MMPLD file must not be a "
            "null pointer.");
    }

    // Open the file.
    outStream.close();
    outStream.open(path, std::ios::binary);

    if (!outStream.is_open()) {
        throw std::runtime_error("MMPLD file could not be opened");
    }

    // Read and check the header.
    outStream.read(reinterpret_cast<char *>(&outHeader), sizeof(outHeader));
    if (::strcmp(outHeader.magic_identifier, "MMPLD") != 0) {
        throw std::runtime_error("The given stream does not start with a valid "
            "MMPLD header.");
    }

    log::instance().write_line(log_level::verbose, "Read MMPLD version {}; "
        "{} frames; data within ({}, {}, {}) - ({}, {}, {}); clipping box "
        "({}, {}, {}) - ({}, {}, {})", outHeader.version, outHeader.frames,
        outHeader.bounding_box[0], outHeader.bounding_box[1],
        outHeader.bounding_box[2], outHeader.bounding_box[3],
        outHeader.bounding_box[4], outHeader.bounding_box[5],
        outHeader.clipping_box[0], outHeader.clipping_box[1],
        outHeader.clipping_box[2], outHeader.clipping_box[3],
        outHeader.clipping_box[4], outHeader.clipping_box[5]);

    // Read the seek table.
    outSeekTable.clear();
    outSeekTable.reserve(outHeader.frames);
    for (uint32_t i = 0; i < outHeader.frames; ++i) {
        outStream.read(reinterpret_cast<char *>(&offset), sizeof(offset));
        outSeekTable.push_back(offset);
    }

    return outStream;
}


/*
 * trrojan::mmpld_reader::read_frame_header
 */
void trrojan::mmpld_reader::read_frame_header(frame_header& outHeader,
        std::ifstream& file, const std::uint16_t version) {
    int major, minor;
    mmpld_reader::parse_version(major, minor, version);

    ::memset(&outHeader, 0, sizeof(outHeader));

    if (minor >= 2) {
        mmpld_reader::read(outHeader.timestamp, file);
    }

    mmpld_reader::read(outHeader.lists, file);

    log::instance().write_line(log_level::verbose, "Read MMPLD frame header: "
        "{} lists; timestamp {}", outHeader.lists, outHeader.timestamp);
}


/*
 * trrojan::mmpld_reader::read_list_header
 */
void trrojan::mmpld_reader::read_list_header(list_header& outHeader,
        std::ifstream& file) {
    ::memset(&outHeader, 0, sizeof(outHeader));

    mmpld_reader::read(outHeader.vertex_type, file);
    mmpld_reader::read(outHeader.colour_type, file);

    switch (outHeader.vertex_type) {
        case vertex_type::float_xyz:
        case vertex_type::short_xyz:
            mmpld_reader::read(outHeader.radius, file);
            break;

        default:
            outHeader.radius = -1.0f;
            break;
    }

    switch (outHeader.colour_type) {
        case colour_type::none: {
            std::uint8_t rgba[4];
            mmpld_reader::read(rgba, file);
            for (size_t i = 0; i < 4; ++i) {
                outHeader.colour[i] = static_cast<float>(rgba[i])
                    / static_cast<float>(UCHAR_MAX);
            }
            outHeader.min_intensity = 0.0f;
            outHeader.max_intensity = -1.0f;
        } break;

        case colour_type::float_i:
            ::memset(&outHeader.colour, 0, sizeof(outHeader.colour));
            mmpld_reader::read(outHeader.min_intensity, file);
            mmpld_reader::read(outHeader.max_intensity, file);
            break;

        default:
            ::memset(&outHeader.colour, 0, sizeof(outHeader.colour));
            outHeader.min_intensity = 0.0f;
            outHeader.max_intensity = -1.0f;
            break;
    }

    mmpld_reader::read(outHeader.particles, file);

    log::instance().write_line(log_level::verbose, "Read MMPLD list header: {}"
        " particles; position type {}; colour type {}; constant colour "
        "({}, {}, {}, {}); intensity range {} - {}; constant radius {}",
        outHeader.particles, static_cast<int>(outHeader.vertex_type),
        static_cast<int>(outHeader.colour_type), outHeader.colour[0],
        outHeader.colour[1], outHeader.colour[2], outHeader.colour[3],
        outHeader.min_intensity, outHeader.max_intensity, outHeader.radius);
}
