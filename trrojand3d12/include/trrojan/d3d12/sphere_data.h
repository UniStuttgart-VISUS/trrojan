// <copyright file="sphere_data.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <unordered_map>

#include <mmpld.h>

#include "trrojan/camera.h"
#include "trrojan/random_sphere_generator.h"

#include "trrojan/d3d12/sphere_rendering_configuration.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Composable container for MMPLD and random sphere data.
    /// </summary>
    class TRROJAND3D12_API sphere_data final {

    public:

        /// <summary>
        /// A descriptor table repesented as a descriptor heap and offsets to
        /// the entries in that heap.
        /// </summary>
        typedef std::pair<ATL::CComPtr<ID3D12DescriptorHeap>,
            std::vector<D3D12_GPU_DESCRIPTOR_HANDLE>> descriptor_table_type;

        /// <summary>
        /// A type to express the properties of a particle data set as a
        /// bitmask.
        /// </summary>
        typedef random_sphere_generator::properties_type properties_type;

        /// <summary>
        /// The underlying bitmask type of <see cref="properties_type" />.
        /// </summary>
        typedef std::underlying_type<properties_type>::type property_mask_type;

        /// <summary>
        /// An identifier for a single shader variant.
        /// </summary>
        typedef sphere_rendering_configuration::shader_id_type shader_id_type;

        /// <summary>
        /// Answer whether the given type of particle colour is a non-floating
        /// point per-sphere colour type.
        /// </summary>
        static bool is_non_float_colour(const mmpld::colour_type colour);

        /// <summary>
        /// Answer whether the given MMPLD particles list uses a non-floating
        /// point per-sphere colour type.
        /// </summary>
        /// <param name="header"></param>
        /// <returns></returns>
        inline static bool is_non_float_colour(
                const mmpld::list_header& header) {
            return is_non_float_colour(header.colour_type);
        }

        /// <summary>
        /// Parses the data set in the given <see cref="configuration" /> as
        /// configuration of random spheres.
        /// </summary>
        /// <param name="config"></param>
        /// <param name="shader_code"></param>
        /// <returns></returns>
        static random_sphere_generator::description parse_random_sphere_desc(
            const sphere_rendering_configuration& config,
            const shader_id_type shader_code);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        sphere_data(void);

        /// <summary>
        /// Computes the final shader code from what has been requested by the
        /// configuration and the data that have been loaded.
        shader_id_type adjust_shader_code(shader_id_type shader_code) const;

        /// <summary>
        /// Answer the extrema of the bounding box of the data set.
        /// </summary>
        inline const std::array<glm::vec3, 2>& bbox(void) const noexcept {
            return this->_bbox;
        }

        /// <summary>
        /// Gets the end point of the bounding box.
        /// </summary>
        inline const glm::vec3& bbox_end(void) const noexcept {
            return this->_bbox[1];
        }

        /// <summary>
        /// Gets the start point of the bounding box.
        /// </summary>
        inline const glm::vec3& bbox_start(void) const noexcept {
            return this->_bbox[0];
        }

        /// <summary>
        /// Clears any loaded data.
        /// </summary>
        void clear(void);

        /// <summary>
        /// Answer the global colour of the spheres, if any.
        /// </summary>
        inline const std::array<float, 4>& colour(void) const noexcept {
            return this->_colour;
        }

        /// <summary>
        /// Answer the buffer holding the read-only data.
        /// </summary>
        /// <remarks>
        /// This buffer does not necessarily need to be valid, because callers
        /// can chose to load data into a custom buffer.
        /// </remarks>
        inline ATL::CComPtr<ID3D12Resource> data(void) const noexcept {
            return this->_data;
        }

        /// <summary>
        /// Answer the size of the bounding box of the currently loaded data
        /// set.
        /// </summary>
        std::array<float, 3> extents(void) const;

        /// <summary>
        /// Retreives the constant buffer data for the current data properties.
        /// </summary>
        /// <param name="out_constants"></param>
        void get_sphere_constants(SphereConstants& out_constants) const;

        /// <summary>
        /// Answer the description of the input layout required for creating a
        /// vertex buffer from <see cref="data" />.
        /// </summary>
        inline const std::vector<D3D12_INPUT_ELEMENT_DESC>& input_layout(
                void) const noexcept {
            return this->_input_layout;
        }

        /// <summary>
        /// Answer the range of intensity values if the spheres have scalars
        /// attached.
        /// </summary>
        inline const std::array<float, 2>& intensity_range(
                void) const noexcept {
            return this->_intensity_range;
        }

        /// <summary>
        /// Load the data set into an immutable buffer and transition this
        /// buffer for use as shader resource.
        /// </summary>
        /// <remarks>
        /// <para>The buffer returned is an upload buffer for data that will be
        /// in <paramref name="state" /> after <paramref name="cmd_list" /> was
        /// executed. Callers need to run the command list before using the data
        /// set. The benchmark class will keep an copy of this buffer for
        /// creating the appropriate shader resource views or input assembler
        /// bindings, but it will not retain the upload buffer. If the caller
        /// releases it, it will be freed.
        /// </para>
        /// <para>If 
        /// <see cref="sphere_rendering_configuration::factor_fit_bounding_box" />
        ///  and an MMPLD file is loaded, this method will recompute the
        /// bounding box when loading it.</para>
        /// </remarks>
        /// <param name="cmd_list"></param>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        /// <returns>A pointer the upload buffer. Callers need to keep this
        /// buffer alive until they have executed <paramref name="cmd_list" />.
        /// </returns>
        ATL::CComPtr<ID3D12Resource> load(
            ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code,
            const sphere_rendering_configuration& config,
            const D3D12_RESOURCE_STATES state);

        /// <summary>
        /// Load the data set into the memory provided by
        /// <paramref name="allocator" />.
        /// </summary>
        /// <remarks>
        /// <para>If 
        /// <see cref="sphere_rendering_configuration::factor_fit_bounding_box" />
        /// and an MMPLD file is loaded, this method will recompute the bounding
        /// box when loading it.
        /// </para>
        /// </remarks>
        /// <param name="allocator"></param>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        /// <returns>The size, in bytes, of the data buffer. This is the same
        /// value that is also passed to <paramref name="allocator" />.</returns>
        UINT64 load(std::function<void *(const UINT64)> allocator,
            const shader_id_type shader_code,
            const sphere_rendering_configuration& config);

        /// <summary>
        /// Load the data set properties and input layout for the given
        /// configuration, but not the data itself.
        /// </summary>
        /// <remarks>
        /// <para>The <see cref="factor_fit_bounding_box" /> will not be
        /// honoured by this method, because the data need to be loaded to fit
        /// the bounding box. In turn, this method will set the bounding box
        /// from the list header in the file in any case.</para>
        /// <para>If loading an MMPLD file, the maximum radius retrieved is
        /// only correct for data sets with global radius. In any other case,
        /// the radius can only be retrieved by actually loading the data.
        /// </para>
        /// </remarks>
        /// <param name="shader_code"></param>
        /// <param name="config"></param>
        void load_properties(const shader_id_type shader_code,
            const sphere_rendering_configuration& config);

        /// <summary>
        /// Answer the maximum radius of any sphere in the data set.
        /// </summary>
        inline float max_radius(void) const noexcept {
            return this->_max_radius;
        }

        /// <summary>
        /// Answer the property mask of the data set.
        /// </summary>
        inline properties_type properties(void) const noexcept {
            return this->_properties;
        }

        /// <summary>
        /// Retrieves the properties of the currently loaded data set and erases
        /// all flags which are not relevant for the given shader technique.
        /// </summary>
        property_mask_type properties(const shader_id_type shader_code) const;

        /// <summary>
        /// Answer the number of spheres in the data set.
        /// </summary>
        inline UINT spheres(void) const noexcept {
            return this->_cnt_spheres;
        }

        /// <summary>
        /// Answer the stride of the spheres in bytes.
        /// </summary>
        inline UINT stride(void) const noexcept {
            return this->_stride;
        }

        /// <summary>
        /// Answer whether the data pointer in this object is valid, ie if there
        /// are data loaded on the GPU.
        /// </summary>
        inline operator bool(void) const noexcept {
            return (this->_data != nullptr);
        }

    private:

        static const property_mask_type property_float_colour;
        static const property_mask_type property_per_sphere_colour;
        static const property_mask_type property_per_sphere_intensity;
        static const property_mask_type property_per_sphere_radius;
        static const property_mask_type property_structured_resource;

        /// <summary>
        /// Fits the stored bounding box to the actual data in the given MMPLD
        /// particle list.
        /// </summary>
        /// <param name="header"></param>
        /// <param name="particles"></param>
        void fit_bounding_box(const mmpld::list_header &header,
            const void *particles);

        /// <summary>
        /// Determines the actual maximum radius of any sphere in the given
        /// MMPLD particle list and persists it.
        /// </summary>
        /// <remarks>
        /// This method will check the whole particle list in case the header
        /// indicates varying radii. Otherwise, the data from the header is
        /// used.
        /// </remarks>
        /// <param name="header"></param>
        /// <param name="particles"></param>
        void set_max_radius(const mmpld::list_header& header,
            const void *particles);

        /// <summary>
        /// Persists the data properties of the given random-sphere data set.
        /// </summary>
        /// <param name="desc"></param>
        void set_properties(const random_sphere_generator::description& desc);

        /// <summary>
        /// Persists the data properties of the given MMPLD particle list.
        /// </summary>
        /// <param name="file_header"></param>
        /// <param name="list_header"></param>
        void set_properties(const mmpld::file_header& file_header,
            const mmpld::list_header& list_header);

        std::array<glm::vec3, 2> _bbox;
        UINT _cnt_spheres;
        std::array<float, 4> _colour;
        ATL::CComPtr<ID3D12Resource> _data;
        std::vector<D3D12_INPUT_ELEMENT_DESC> _input_layout;
        std::array<float, 2> _intensity_range;
        float _max_radius;
        properties_type _properties;
        UINT _stride;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
