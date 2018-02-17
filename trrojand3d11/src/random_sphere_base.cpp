/// <copyright file="random_sphere_base.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/random_sphere_base.h"

#include <cassert>
#include <cinttypes>
#include <random>

#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::d3d11::random_sphere_base::get_random_input
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::random_sphere_base::get_random_input(
        const random_sphere_type type) {
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> _INT = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> _RGBA8 = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0,  16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    static const std::vector<D3D11_INPUT_ELEMENT_DESC> _RGBA32 = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };


    switch (type) {
        case random_sphere_type::pos_intensity:
        case random_sphere_type::pos_rad_intensity:
            return _INT;

        case random_sphere_type::pos_rgba8:
        case random_sphere_type::pos_rad_rgba8:
            return _RGBA8;

        case random_sphere_type::pos_rgba32:
        case random_sphere_type::pos_rad_rgba32:
            return _RGBA32;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }
}


/*
 * trrojan::d3d11::random_sphere_base::get_random_sphere_stride
 */
size_t trrojan::d3d11::random_sphere_base::get_random_sphere_stride(
        const random_sphere_type type) {
    switch (type) {
        case random_sphere_type::pos_intensity:
        case random_sphere_type::pos_rad_intensity:
            return sizeof(random_sphere_intensity);

        case random_sphere_type::pos_rgba8:
        case random_sphere_type::pos_rad_rgba8:
            return sizeof(random_sphere_rgba8);

        case random_sphere_type::pos_rgba32:
        case random_sphere_type::pos_rad_rgba32:
            return sizeof(random_sphere_rgba32);

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }
}


/*
 * trrojan::d3d11::random_sphere_base::make_random_spheres
 */
trrojan::d3d11::rendering_technique::buffer_type
trrojan::d3d11::random_sphere_base::make_random_spheres(ID3D11Device *device,
        const buffer_type bufferType,
        const random_sphere_type sphereType,
        const std::uint32_t cntParticles,
        const std::array<float, 3>& domainSize,
        const std::array<float, 2>& sphereSize,
        const std::uint32_t seed) {
    D3D11_BUFFER_DESC bufferDesc;
    D3D11_SUBRESOURCE_DATA id;
    std::uniform_real_distribution<float> posDist(0, 1);
    std::uniform_real_distribution<float> radDist(sphereSize[0], sphereSize[1]);
    std::vector<std::uint8_t> particles;
    std::mt19937 prng;
    rendering_technique::buffer_type retval;
    size_t stride = random_sphere_base::get_random_sphere_stride(sphereType);

    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device to create the vertex "
            "buffer on must not be nullptr.");
    }

    prng.seed(seed);
    particles.resize(cntParticles * stride);

    log::instance().write_line(log_level::verbose, "Creating %u random "
        "sphere(s) of type %u on a domain of [%f, %f, %f] with a uniformly "
        "distributed size in [%f, %f]. The random seed is %u.", cntParticles,
        static_cast<std::uint32_t>(sphereType), domainSize[0], domainSize[1],
        domainSize[2], sphereSize[0], sphereSize[1], seed);

    for (std::uint32_t i = 0; i < cntParticles; ++i) {
        auto p = particles.data() + (i * stride);
        auto g = static_cast<float>(i) / static_cast<float>(cntParticles);
        auto pos = reinterpret_cast<DirectX::XMFLOAT4 *>(p);

        pos->x = posDist(prng) * domainSize[0] - 0.5f * domainSize[0];
        pos->y = posDist(prng) * domainSize[1] - 0.5f * domainSize[1];
        pos->z = posDist(prng) * domainSize[2] - 0.5f * domainSize[2];

        switch (sphereType) {
            case random_sphere_type::pos_rad_intensity:
            case random_sphere_type::pos_rad_rgba32:
            case random_sphere_type::pos_rad_rgba8:
                pos->w = radDist(prng);
                break;

            default:
                pos->w = 0.0f;
        }

        switch (sphereType) {
            case random_sphere_type::pos_intensity:
            case random_sphere_type::pos_rad_intensity:
                *reinterpret_cast<float *>(pos + 1) = g;
                break;

            case random_sphere_type::pos_rgba32:
            case random_sphere_type::pos_rad_rgba32:
                *reinterpret_cast<float *>(pos + 1) = g;
                *reinterpret_cast<float *>(pos + 2) = g;
                *reinterpret_cast<float *>(pos + 3) = g;
                *reinterpret_cast<float *>(pos + 4) = 1.0f;
                break;

            case random_sphere_type::pos_rgba8:
            case random_sphere_type::pos_rad_rgba8:
                *reinterpret_cast<COLORREF *>(pos + 1)
                    = RGB(g * 255, g * 255, g * 255);
                break;

            default:
                throw std::runtime_error("Unexpected sphere format.");
        }
    }

    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = static_cast<UINT>(cntParticles * stride);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.CPUAccessFlags = 0;
    switch (bufferType) {
        case buffer_type::vertex_buffer:
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;

        case buffer_type::structured_resource:
            bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            bufferDesc.StructureByteStride = static_cast<UINT>(stride);
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
            break;

        default:
            throw std::runtime_error("An invalid Direct3D buffer type was "
                "specified for creating random spheres.");
    }

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
trrojan::d3d11::rendering_technique::buffer_type
trrojan::d3d11::random_sphere_base::make_random_spheres(ID3D11Device *device,
        const buffer_type bufferType, const std::string& configuration) {
    static const std::runtime_error PARSE_ERROR("The configuration description "
        "of the random spheres is invalid. The configuration must have the "
        "following format: \"<sphere type> : <number of spheres> : <random "
        "seed or \"-\"> : <domain size> : <sphere size range>\"");
    static const char SEPARATOR = ':';
#define _ADD_SPHERE_TYPE(n) { #n, random_sphere_type::n }
    static const struct {
        const char *name;
        random_sphere_type type;
    } SPHERE_TYPES[] = {
        _ADD_SPHERE_TYPE(pos_intensity),
        _ADD_SPHERE_TYPE(pos_rgba32),
        _ADD_SPHERE_TYPE(pos_rgba8),
        _ADD_SPHERE_TYPE(pos_rad_intensity),
        _ADD_SPHERE_TYPE(pos_rad_rgba32),
        _ADD_SPHERE_TYPE(pos_rad_rgba8)
    };
#undef _ADD_SPHERE_TYPE

    std::uint32_t cntParticles = 0;
    std::array<float, 3> domainSize = { 0, 0, 0 };
    std::uint32_t seed = 0;
    std::array<float, 2> sphereSize = { 0 ,0 };
    random_sphere_type sphereType = unspecified;

    /* Parse the type of spheres to generate. */
    auto tokBegin = configuration.begin();
    auto tokEnd = std::find(configuration.begin(), configuration.end(),
        SEPARATOR);
    if (tokEnd == configuration.end()) {
        throw PARSE_ERROR;
    }

    auto token = tolower(trim(std::string(tokBegin, tokEnd)));
    for (auto& t : SPHERE_TYPES) {
        if (token == t.name) {
            sphereType = t.type;
            break;
        }
    }
    if (sphereType == random_sphere_type::unspecified) {
        throw std::runtime_error("The type of random spheres to generate is "
            "invalid.");
    }

    /* Parse the number of spheres to generate. */
    tokBegin = ++tokEnd;
    tokEnd = std::find(tokEnd, configuration.end(), SEPARATOR);
    if (tokEnd == configuration.end()) {
        throw PARSE_ERROR;
    }

    cntParticles = parse<decltype(cntParticles)>(std::string(tokBegin, tokEnd));

    /* Parse the random seed. */
    tokBegin = ++tokEnd;
    tokEnd = std::find(tokEnd, configuration.end(), SEPARATOR);
    if (tokEnd == configuration.end()) {
        throw PARSE_ERROR;
    }

    try {
        seed = parse<decltype(seed)>(std::string(tokBegin, tokEnd));
    } catch (...) {
        // Special case: use "real" random seed.
        std::random_device rnd;
        seed = rnd();
    }

    /* Parse the size of the domain. */
    tokBegin = ++tokEnd;
    tokEnd = std::find(tokEnd, configuration.end(), SEPARATOR);
    if (tokEnd == configuration.end()) {
        throw PARSE_ERROR;
    }

    domainSize = parse<decltype(domainSize)>(std::string(tokBegin, tokEnd));

    /* Parse the range of possible sphere sizes. */
    tokBegin = ++tokEnd;
    tokEnd = configuration.end();
    sphereSize = parse<decltype(sphereSize)>(std::string(tokBegin, tokEnd));

    return this->make_random_spheres(device, bufferType, sphereType,
        cntParticles, domainSize, sphereSize, seed);
}
