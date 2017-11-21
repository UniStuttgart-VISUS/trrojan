/// <copyright file="random_sphere_base.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <vector>

#include <Windows.h>
#include <atlbase.h>
#include <atlcom.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "trrojan/configuration.h"
#include "trrojan/factor.h"

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Implements a random particle creator.
    /// </summary>
    class TRROJAND3D11_API random_sphere_base {

    public:

        static const std::string factor_domain_size;
        static const std::string factor_number_of_particles;
        static const std::string factor_seed;
        static const std::string factor_sphere_size;

        /// <summary>
        /// Creates a vector format descriptor for the random spheres.
        /// </summary>
        static std::vector<D3D11_INPUT_ELEMENT_DESC> get_random_input(void);

        virtual ~random_sphere_base(void) = default;

    protected:

        struct random_sphere {
            DirectX::XMFLOAT4 Position;
            COLORREF Colour;
        };

        random_sphere_base(void) = default;

        ATL::CComPtr<ID3D11Buffer> make_random_spheres(ID3D11Device *device,
            const std::uint32_t cntParticles,
            const std::array<float, 3>& domainSize,
            const std::array<float, 2>& sphereSize,
            const std::uint32_t seed);

        ATL::CComPtr<ID3D11Buffer> make_random_spheres(ID3D11Device *device,
            const configuration& config);

    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
