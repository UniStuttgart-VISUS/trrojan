/// <copyright file="mmpld_data_set.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d11/sphere_data_set.h"

#include "trrojan/mmpld_reader.h"



namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Base class for using MMPLD files.
    /// </summary>
    class TRROJAND3D11_API mmpld_data_set : public sphere_data_set_base {

    public:

        /// <summary>
        /// The possible colour types from MMPLD.
        /// </summary>
        typedef mmpld_reader::colour_type colour_type;

        /// <summary>
        /// The type of flags controlling the loader.
        /// </summary>
        typedef sphere_data_set_base::properties_type frame_load_flags;

        /// <summary>
        /// The possible types of spheres from MMPLD files.
        /// </summary>
        typedef mmpld_reader::vertex_type sphere_type;

        /// <summary>
        /// Open the given MMPLD data set without reading a frame.
        /// </summary>
        static sphere_data_set create(const char *path);

        /// <summary>
        /// Open the given MMPLD data set without reading a frame.
        /// </summary>
        inline static sphere_data_set create(const std::string& path) {
            return mmpld_data_set::create(path.c_str());
        }

        /// <summary>
        /// Creates a vertex input layout descriptor for the given MMPLD list
        /// header.
        /// </summary>
        /// <param name="header">The header of the particle list to get the
        /// vertex format for.</param>
        /// <returns>The vertex descriptor.</returns>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> get_input_layout(
            const mmpld_reader::list_header& header);

        /// <summary>
        /// Retrieves the data properties inherent to the spheres in the given
        /// list.
        /// </summary>
        static sphere_data_set_base::properties_type get_properties(
            const mmpld_reader::list_header& header);

        virtual ~mmpld_data_set(void) = default;

        /// <inheritdoc />
        virtual void bounding_box(point_type& outMin, point_type& outMax) const;

        /// <summary>
        /// Provides access to the list header of the current frame, which
        /// contains information like a global colour or radius.
        /// </summary>
        inline const mmpld_reader::list_header& header(void) const {
            return this->_list;
        }

        /// <inheritdoc />
        virtual float max_radius(void) const;

        /// <summary>
        /// Read the given frame from the previously opened MMPLD stream and
        /// upload it to a vertex buffer on the given device.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="frame"></param>
        /// <param name="options">Controls certain properties of the data being
        /// generated. Valid bits are
        /// <see cref="sphere_data_set::property_float_colour" /> for forcing
        /// 8-bit colour channels to float and
        /// <see cref="sphere_data_set::property_structured_resource" /> for
        /// creating a structured resource buffer instead of a vertex buffer.
        /// </param>
        /// <returns></returns>
        rendering_technique::buffer_type read_frame(ID3D11Device *device,
            const unsigned int frame, const frame_load_flags options);

        /// <inheritdoc />
        virtual size_type size(void) const;

        /// <inheritdoc />
        virtual size_type stride(void) const;

    protected:

        /// <summary>
        /// The bitmask describing the input properties.
        /// </summary>
        typedef std::underlying_type<mmpld_reader::shader_properties>::type
            mmpld_input_properties;

        /// <summary>
        /// Get the the colour semantic in the given input layout range.
        /// </summary>
        template<class I>
        static I get_colour_offset(I&& begin, I&& end) {
            return std::find_if(begin, end, [](D3D11_INPUT_ELEMENT_DESC& d) { 
                return (::strcmp(d.SemanticName, "COLOR") == 0);
            });
        }

        /// <summary>
        /// Answer whether the colour format of the given list is not a floating
        /// point format.
        /// </summary>
        static bool is_non_float_colour(const mmpld_reader::list_header& list);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        mmpld_data_set(void);

        /// <summary>
        /// Determine the input features of the <see cref="mmpld_list" />.
        /// </summary>
        /// <returns></returns>
        mmpld_input_properties get_mmpld_input_properties(void) const;

        /// <summary>
        /// Opens the MMPLD file at the specified location and reads its header
        /// and seek table.
        /// </summary>
        /// <param name="path"></param>
        /// <returns>The state of the stream after the above-mentioned
        /// operations have been performed.</returns>
        bool open(const char *path);

        /// <summary>
        /// The header of the currently opened MMPLD file.
        /// </summary>
        /// <remarks>
        /// This variable is only valid while <see cref="_stream" /> is
        /// good. Otherwise, its content might be bogus.
        /// </remarks>
        mmpld_reader::file_header _header;

        /// <summary>
        /// The list header of the last read MMPLD particle list.
        /// </summary>
        mmpld_reader::list_header _list;

        /// <summary>
        /// Radius used for clipping computation of the current particle list.
        /// </summary>
        float _max_radius;

        /// <summary>
        /// The seek table for the currently opened MMPLD file.
        /// </summary>
        /// <remarks>
        /// This variable is only valid while <see cref="_stream" /> is
        /// good. Otherwise, its content might be bogus.
        /// </remarks>
        mmpld_reader::seek_table _seek_table;

        /// <summary>
        /// The stream of the currently opened MMPLD file.
        /// </summary>
        std::ifstream _stream;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
