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
        /// The type of GPU resources allocated for the data set.
        /// </summary>
        enum buffer_type {
            /// <summary>
            /// The data set was allocated as vertex buffer.
            /// </summary>
            vertex_buffer,

            /// <summary>
            /// The data set was allocated as structured resource.
            /// </summary>
            structured_resource
        };

        /// <summary>
        /// The data type for 3D points.
        /// </summary>
        typedef trrojan::graphics_benchmark_base::point_type point_type;

        /// <summary>
        /// Define a Direct3D-compatible size type.
        /// </summary>
        typedef std::uint32_t size_type;

        virtual ~sphere_data_set_base(void) = default;

        /// <summary>
        /// Gets the two opposite points defining the bounding box of the data
        /// set.
        /// </summary>
        virtual void bounding_box(point_type& outMin,
            point_type& outMax) const = 0;

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
        /// Gets the stride of the particles (in bytes).
        /// </summary>
        virtual size_type stride(void) const = 0;

        /// <summary>
        /// Answer the number of particles in the buffer.
        /// </summary>
        virtual size_type size(void) const = 0;

    protected:

        inline sphere_data_set_base(void) = default;

        /// <summary>
        /// The buffer holding the data.
        /// </summary>
        rendering_technique::buffer_type _buffer;

        /// <summary>
        /// Vertex layout descriptor for the data.
        /// </summary>
        std::vector<D3D11_INPUT_ELEMENT_DESC> _layout;

    };


    /// <summary>
    /// Pointer to a generic sphere data set.
    /// </summary>
    typedef std::shared_ptr<sphere_data_set_base> sphere_data_set;

} /* end namespace d3d11 */
} /* end namespace trrojan */
