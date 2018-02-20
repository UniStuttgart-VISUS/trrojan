/// <copyright file="sphere_data_set.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <memory>
#include <vector>

#include <Windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "hlsltypemapping.hlsli"

#include "trrojan/camera.h"
#include "trrojan/graphics_benchmark_base.h"

#include "trrojan/d3d11/export.h"
#include "trrojan/d3d11/rendering_technique.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// The base class for the Direct3D resource of a sphere data type.
    /// </summary>
    class TRROJAND3D11_API sphere_data_set_base {

    public:

        /// <summary>
        /// The data type for 3D points.
        /// </summary>
        typedef trrojan::graphics_benchmark_base::point_type point_type;

        /// <summary>
        /// Data type to specify data set properties.
        /// </summary>
        /// <remarks>
        /// The properties are stored in a bitfield which is compatible with
        /// the flags of the shader techniques used by the
        /// <see cref="sphere_benchmark" />.
        /// </remarks>
        typedef std::uint64_t properties_type;

        /// <summary>
        /// Define a Direct3D-compatible size type.
        /// </summary>
        typedef std::uint32_t size_type;

        static const properties_type property_float_colour;
        static const properties_type property_per_sphere_colour;
        static const properties_type property_per_sphere_intensity;
        static const properties_type property_per_sphere_radius;
        static const properties_type property_structured_resource;

        virtual ~sphere_data_set_base(void) = default;

        /// <summary>
        /// Applies the data set as vertex buffer or shader resource in the
        /// given <see cref="rendering_technique" />.
        /// </summary>
        /// <param name="technique">The <see cref="rendering_technique" /> to
        /// add the data to.</param>
        /// <param name="stages">If the data set used a structured resouce,
        /// bind it to the specified stages. If the data set is a vertex buffer,
        /// this parameter has no effect.</param>
        /// <param name="idxVb">If the data set is a vertex buffer, bind it to
        /// this slot. If it is no vertex buffer, this parameter has no effect.
        /// </param>
        /// <param name="idxSrv">If the data set is a structured resource
        /// buffer, bind it to this slot. Otherwise, this parameter has no
        /// effect.</param>
        void apply(rendering_technique& technique,
            const rendering_technique::shader_stages stages,
            const UINT idxVb = 0, const UINT idxSrv = 0);

        /// <summary>
        /// Gets the two opposite points defining the bounding box of the data
        /// set.
        /// </summary>
        virtual void bounding_box(point_type& outMin,
            point_type& outMax) const = 0;

        /// <summary>
        /// Answer the buffer holding the data.
        /// </summary>
        inline const rendering_technique::buffer_type& buffer(void) const {
            return this->_buffer;
        }

        /// <summary>
        /// Gets the world-space centre point of the current data set.
        /// </summary>
        point_type centre(void) const;

        /// <summary>
        /// Make a suggestion for near and far clipping plane for the data set
        /// and the given <see cref="trrojan::camera" />.
        /// </summary>
        std::pair<float, float> clipping_planes(const camera& cam) const;

        /// <summary>
        /// Gets the size of the bounding box.
        /// </summary>
        std::array<float, 3> extents(void) const;

        /// <summary>
        /// Gets the input layout descriptor for the data set.
        /// </summary>
        inline const std::vector<D3D11_INPUT_ELEMENT_DESC> layout(void) const {
            return this->_layout;
        }

        /// <summary>
        /// Gets the maximum radius of any sphere in the data set.
        /// </summary>
        virtual float max_radius(void) const = 0;

        /// <summary>
        /// Gets certain graphics-relevant properties of the data set.
        /// </summary>
        inline properties_type properties(void) const {
            return this->_properties;
        }

        /// <summary>
        /// Gets the stride of the particles (in bytes).
        /// </summary>
        virtual size_type stride(void) const = 0;

        /// <summary>
        /// Answer the number of particles in the buffer.
        /// </summary>
        virtual size_type size(void) const = 0;

    protected:

        inline sphere_data_set_base(void) : _properties(0) { }

        /// <summary>
        /// The buffer holding the data.
        /// </summary>
        rendering_technique::buffer_type _buffer;

        /// <summary>
        /// Vertex layout descriptor for the data.
        /// </summary>
        std::vector<D3D11_INPUT_ELEMENT_DESC> _layout;

        /// <summary>
        /// Holds the properties of the data set.
        /// </summary>
        properties_type _properties;
    };


    /// <summary>
    /// Pointer to a generic sphere data set.
    /// </summary>
    typedef std::shared_ptr<sphere_data_set_base> sphere_data_set;

} /* end namespace d3d11 */
} /* end namespace trrojan */
