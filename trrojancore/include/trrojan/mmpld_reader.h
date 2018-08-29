/// <copyright file="mmpld_reader.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/export.h"
#include "trrojan/particle_properties.h"

#include <cstdint>
#include <fstream>
#include <type_traits>
#include <vector>


namespace trrojan {

    /// <summary>
    /// Provides help for reading MegaMol binary particle files (mmpld).
    /// </summary>
    /// <remarks>
    /// See https://svn.vis.uni-stuttgart.de/trac/megamol/wiki/mmpld.
    /// The reader does not support three-component colour formats and
    /// the 16-bit position format.
    /// </remarks>
    class TRROJANCORE_API mmpld_reader {

    public:

        /// <summary>
        /// The type of the per-vertex data stored in the file.
        /// </summary>
        enum class vertex_type : std::uint8_t {

            /// <summary>
            /// No positions are available on a per-vertex base.
            /// </summary>
            none = 0,

            /// <summary>
            /// 3D position.
            /// </summary>
            float_xyz = 1,

            /// <summary>
            /// 3D position and radius.
            /// </summary>
            float_xyzr = 2,

            /// <summary>
            /// 16-bit 3D position.
            /// </summary>
            short_xyz = 3
        };

        /// <summary>
        /// The type of the colour data stored in the file.
        /// </summary>
        enum class colour_type : std::uint8_t {
            none = 0,
            uint8_rgb = 1,
            uint8_rgba = 2,
            float_i = 3,
            float_rgb = 4,
            float_rgba = 5
        };

        typedef particle_properties shader_properties;

#pragma pack(push, 1)
        /// <summary>
        /// The header at the beginning of any mmpld file.
        /// </summary>
        struct file_header {
            char magic_identifier[6];
            std::uint16_t version;
            std::uint32_t frames;
            float bounding_box[6];
            float clipping_box[6];
        };
        static_assert(sizeof(file_header) == 60, "FileHeader is correctly "
            "aligned to match the implementation in MegaMol.");
#pragma pack(pop)

        static_assert(sizeof(vertex_type) == 1, "vertex_type memory footprint "
            "is one byte.");
        static_assert(sizeof(colour_type) == 1, "colour_type memory footprint "
            "is one byte.");

        /// <summary>
        /// The descriptor of a frame, which might comprise multiple lists.
        /// </summary>
        struct frame_header {
            /// <summary>
            /// The number of lists in the frame.
            /// </summary>
            std::int32_t lists;

            /// <summary>
            /// The (optional) timestamp of the frame.
            /// </summary>
            float timestamp;
        };

        /// <summary>
        /// The descriptor of the content of a particle list.
        /// </summary>
        struct list_header {

            /// <summary>
            /// The constant colour of the particles if
            /// <see cref="list_header::colour_type" /> is
            /// <see cref="colour_type::none" />.
            /// </summary>
            float colour[4];

            /// <summary>
            /// The type of per-vertex colours in the list.
            /// </summary>
            trrojan::mmpld_reader::colour_type colour_type;

            /// <summary>
            /// The maximum intensity found in the list if
            /// <see cref="list_header::colour_type" /> is
            /// <see cref="colour_type::float_i" />.
            /// </summary>
            float max_intensity;

            /// <summary>
            /// The minimum intensity found in the list if
            /// <see cref="list_header::colour_type" /> is
            /// <see cref="colour_type::float_i" />.
            /// </summary>
            float min_intensity;

            /// <summary>
            /// The number of particles in the list.
            /// </summary>
            std::uint64_t particles;

            /// <summary>
            /// The global radius of the particles in the list if no individual
            /// radius is given on a per-vertex basis.
            /// </summary>
            float radius;

            /// <summary>
            /// The type of position data per vertex.
            /// </summary>
            trrojan::mmpld_reader::vertex_type vertex_type;
        };

        /// <summary>
        /// The seek table which contains the offsets of individual particle lists
        /// in the file.
        /// </summary>
        typedef std::vector<std::uint64_t> seek_table;

        /// <summary>
        /// Computes the shader properties from the given
        /// <see cref="list_header" />.
        /// </summary>
        static shader_properties calc_shader_properties(const list_header& header);

        /// <summary>
        /// Computes the particle stride from the given <see cref="list_header" />.
        /// </summary>
        static std::uint64_t calc_stride(const list_header& header);

        //static std::vector<D3D11_INPUT_ELEMENT_DESC> ParseListHeader(
        //    const ListHeader& header);

        /// <summary>
        /// Splits the 16-bit representation of the file version into major and
        /// minor version.
        /// </summary>
        static void parse_version(int& outMajor, int& outMinor,
            const std::uint16_t version);

        /// <summary>
        /// Opens the given MMPLD file and reads its header and seek table.
        /// </summary>
        /// <remarks>
        /// The file stream is returned after reading the header and the seek
        /// table. The file pointer is located after these data.
        /// </remarks>
        static std::ifstream& read_file_header(std::ifstream& outStream,
            file_header& outHeader, seek_table& outSeekTable, const char *path);

        /// <summary>
        /// Reads the frame header (of the specified file version) from the
        /// current position in the file.
        /// </summary>
        static void read_frame_header(frame_header& outHeader,
            std::ifstream& file, const std::uint16_t version);

        /// <summary>
        /// Read a <see cref="list_header" /> from the current position in the
        /// file stream.
        /// </summary>
        static void read_list_header(list_header& outHeader,
            std::ifstream& file);

        mmpld_reader(void) = delete;

        ~mmpld_reader(void) = delete;

    private:

        template<class T> static inline T& read(T& dst, std::ifstream& file) {
            auto cnt = sizeof(T);
            file.read(reinterpret_cast<char *>(&dst), cnt);
            return dst;
        }

    };

}
