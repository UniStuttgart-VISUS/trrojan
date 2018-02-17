/// <copyright file="random_sphere_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 -2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include <Windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "hlsltypemapping.hlsli"

#include "trrojan/configuration.h"
#include "trrojan/factor.h"

#include "trrojan/d3d11/export.h"
#include "trrojan/d3d11/rendering_technique.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a random particle creator.
    /// </summary>
    class TRROJAND3D11_API random_sphere_base {

    public:

        virtual ~random_sphere_base(void) = default;

    protected:

        enum buffer_type {
            vertex_buffer,
            structured_resource
        };

        enum random_sphere_type {
            unspecified,
            pos_intensity,
            pos_rgba32,
            pos_rgba8,
            pos_rad_intensity,
            pos_rad_rgba32,
            pos_rad_rgba8
        };

        struct random_sphere_intensity {
            DirectX::XMFLOAT4 Position;
            float Intensity;
        };

        struct random_sphere_rgba32 {
            DirectX::XMFLOAT4 Position;
            DirectX::XMFLOAT4 Colour;
        };

        struct random_sphere_rgba8 {
            DirectX::XMFLOAT4 Position;
            COLORREF Colour;
        };

        /// <summary>
        /// Creates a vector format descriptor for the random spheres.
        /// </summary>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> get_random_input(
            const random_sphere_type type);

        /// <summary>
        /// Gets the stride of random spheres of the given type.
        /// </summary>
        static size_t get_random_sphere_stride(
            const random_sphere_type type);

        random_sphere_base(void) = default;

        /// <summary>
        /// Creates a vertex or resource buffer filled with random spheres
        /// generated according to the given parameters.
        /// </summary>
        rendering_technique::buffer_type make_random_spheres(
            ID3D11Device *device,
            const buffer_type bufferType,
            const random_sphere_type sphereType,
            const std::uint32_t cntParticles,
            const std::array<float, 3>& domainSize,
            const std::array<float, 2>& sphereSize,
            const std::uint32_t seed);

        rendering_technique::buffer_type make_random_spheres(
            ID3D11Device *device, const buffer_type bufferType,
            const std::string& configuration);

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
