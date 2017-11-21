/// <copyright file="random_sphere_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/random_sphere_base.h"

#include <cassert>
#include <cinttypes>
#include <random>


#define _RANDOM_SPHERE_DEFINE_FACTOR(f)                                        \
const std::string trrojan::d3d11::random_sphere_base::factor_##f(#f)

_RANDOM_SPHERE_DEFINE_FACTOR(domain_size);
_RANDOM_SPHERE_DEFINE_FACTOR(number_of_particles);
_RANDOM_SPHERE_DEFINE_FACTOR(seed);
_RANDOM_SPHERE_DEFINE_FACTOR(sphere_size);

#undef _RANDOM_SPHERE_DEFINE_FACTOR


/*
 * trrojan::d3d11::random_sphere_base::get_random_input
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::random_sphere_base::get_random_input(void) {
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> retval = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,  16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    return retval;
}


/*
 * trrojan::d3d11::random_sphere_base::make_random_spheres
 */
ATL::CComPtr<ID3D11Buffer>
trrojan::d3d11::random_sphere_base::make_random_spheres(ID3D11Device *device,
        const std::uint32_t cntParticles,
        const std::array<float, 3>& domainSize,
        const std::array<float, 2>& sphereSize,
        const std::uint32_t seed) {
    D3D11_BUFFER_DESC bufferDesc;
    D3D11_SUBRESOURCE_DATA id;
    std::uniform_real_distribution<float> posDist(0, 1);
    std::uniform_real_distribution<float> radDist(domainSize[0], domainSize[1]);
    std::vector<random_sphere> particles;
    std::mt19937 prng;
    ATL::CComPtr<ID3D11Buffer> retval;

    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device to create the vertex "
            "buffer on must not be nullptr.");
    }

    prng.seed(seed);
    particles.reserve(cntParticles);

    for (UINT i = 0; i < cntParticles; ++i) {
        particles.emplace_back();
        auto& p = particles.back();

        p.Position.x = posDist(prng) * domainSize[0] - 0.5f * domainSize[0];
        p.Position.y = posDist(prng) * domainSize[1] - 0.5f * domainSize[1];
        p.Position.z = posDist(prng) * domainSize[2] - 0.5f * domainSize[2];
        p.Position.w = radDist(prng);

        auto g = static_cast<float>(i) / static_cast<float>(cntParticles) * 255;
        p.Colour = RGB(g, g, g);
    }

    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = sizeof(random_sphere) * cntParticles;
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;

    ::ZeroMemory(&id, sizeof(id));
    id.pSysMem = particles.data();

    {
        auto hr = device->CreateBuffer(&bufferDesc, &id, &retval);
        if (FAILED(hr)) {
            std::stringstream msg;
            msg << "Failed to create vertex buffer with error " << hr
                << std::ends;
            throw std::runtime_error(msg.str());
        }
    }

    return retval;
}


/*
 * trrojan::d3d11::random_sphere_base::make_random_spheres
 */
ATL::CComPtr<ID3D11Buffer>
trrojan::d3d11::random_sphere_base::make_random_spheres(ID3D11Device *device,
        const configuration& config) {
    std::uint32_t seed;
    
    auto cntParticles = config.get<std::uint32_t>(factor_number_of_particles);
    auto domainSize = config.get<std::array<float, 3>>(factor_domain_size);
    auto sphereSize = config.get<std::array<float, 2>>(factor_sphere_size);

    try {
        seed = config.get<std::uint32_t>(factor_seed);
    } catch (...) {
        std::random_device rnd;
        seed = rnd();
    }

    return this->make_random_spheres(device, cntParticles, domainSize,
        sphereSize, seed);
}
