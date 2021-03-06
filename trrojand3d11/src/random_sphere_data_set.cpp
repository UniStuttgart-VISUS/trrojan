/// <copyright file="random_sphere_data_set.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#include "trrojan/d3d11/random_sphere_data_set.h"

#include <cassert>
#include <cinttypes>
#include <random>

#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::d3d11::random_sphere_data_set::create
 */
trrojan::d3d11::sphere_data_set
trrojan::d3d11::random_sphere_data_set::create(ID3D11Device *device,
        const create_flags flags,
        const sphere_type sphereType,
        const size_type cntParticles,
        const std::array<float, 3>& domainSize,
        const std::array<float, 2>& sphereSize,
        const std::uint32_t seed) {
    static const create_flags VALID_INPUT_FLAGS // Flags directly copied from user input.
        = sphere_data_set_base::property_structured_resource;
    std::shared_ptr<random_sphere_data_set> retval(
        new random_sphere_data_set());

    // Compute bounding box from domain size.
    for (size_t i = 0; i < std::size(domainSize); ++i) {
        retval->_bbox[0][i] = -0.5f * domainSize[i];
        retval->_bbox[1][i] = 0.5f * domainSize[i];
    }

    retval->_layout = random_sphere_data_set::get_input_layout(sphereType);
    retval->_seed = seed;
    retval->_sphere_size = sphereSize;
    retval->_size = cntParticles;
    retval->_type = sphereType;

    retval->recreate(device, flags);

    return retval;
}


/*
 * trrojan::d3d11::random_sphere_data_set::create
 */
trrojan::d3d11::sphere_data_set
trrojan::d3d11::random_sphere_data_set::create(ID3D11Device *device,
        const create_flags flags, const std::string& configuration) {
    static const std::runtime_error PARSE_ERROR("The configuration description "
        "of the random spheres is invalid. The configuration must have the "
        "following format: \"<sphere type> : <number of spheres> : <random "
        "seed or \"-\"> : <domain size> : <sphere size range>\"");
    static const char SEPARATOR = ':';
#define _ADD_SPHERE_TYPE(n) { #n, sphere_type::n }
    static const struct {
        const char *name;
        sphere_type type;
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
    sphere_type sphereType = unspecified;

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
    if (sphereType == sphere_type::unspecified) {
        throw std::runtime_error("The type of random spheres to generate is "
            "invalid.");
    }
    if ((flags & property_float_colour) != 0) {
        // Force 8-bit colours to float on request.
        switch (sphereType) {
            case sphere_type::pos_rgba8:
                sphereType = sphere_type::pos_rgba32;
                break;

            case sphere_type::pos_rad_rgba8:
                sphereType = sphere_type::pos_rad_rgba32;
                break;
        }
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

    return random_sphere_data_set::create(device, flags, sphereType,
        cntParticles, domainSize, sphereSize, seed);
}


/*
 * trrojan::d3d11::random_sphere_data_set::get_input_layout
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::random_sphere_data_set::get_input_layout(
        const sphere_type type) {
    D3D11_INPUT_ELEMENT_DESC element;
    UINT offset = 0;
    std::vector<D3D11_INPUT_ELEMENT_DESC> retval;

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "POSITION";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba8:
        case sphere_type::pos_rad_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "COLOR";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            element.Format = DXGI_FORMAT_R32_FLOAT;
            offset += 1 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            element.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    return std::move(retval);
}


/*
 * trrojan::d3d11::random_sphere_data_set::get_properties
 */
trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::random_sphere_data_set::get_properties(const sphere_type type) {
    properties_type retval = 0;

    switch (type) {
        case pos_rad_intensity:
        case pos_rad_rgba32:
        case pos_rad_rgba8:
            retval |= property_per_sphere_radius;
            break;
    }

    switch (type) {
        case pos_rgba32:
        case pos_rad_rgba32:
            retval |= property_float_colour;
            /* falls through. */
        case pos_rgba8:
        case pos_rad_rgba8:
            retval |= property_per_sphere_colour;
            break;
    }

    switch (type) {
        case pos_intensity:
        case pos_rad_intensity:
            retval |= property_per_sphere_intensity;
            break;
    }

    return retval;
}


/*
 * trrojan::d3d11::random_sphere_data_set::get_stride
 */
trrojan::d3d11::random_sphere_data_set::size_type
trrojan::d3d11::random_sphere_data_set::get_stride(
        const sphere_type type) {
    size_type retval = 0;

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rgba32:
            retval += 3 * sizeof(float);
            break;

        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba8:
        case sphere_type::pos_rad_rgba32:
            retval += 4 * sizeof(float);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            retval += 1 * sizeof(float);
            break;

        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            retval += 4 * sizeof(byte);
            break;

        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            retval += 4 * sizeof(float);
            break;

        default:
            throw std::runtime_error("Unexpected sphere format.");
    }


    return retval;
}


/*
 * trrojan::d3d11::random_sphere_data_set::bounding_box
 */
void trrojan::d3d11::random_sphere_data_set::bounding_box(point_type& outMin,
        point_type& outMax) const {
    outMin = this->_bbox[0];
    outMax = this->_bbox[1];
}


/*
 * trrojan::d3d11::random_sphere_data_set::max_radius
 */
float trrojan::d3d11::random_sphere_data_set::max_radius(void) const {
    return this->_max_radius;
}


/*
 * trrojan::d3d11::random_sphere_data_set::recreate
 */
void trrojan::d3d11::random_sphere_data_set::recreate(ID3D11Device *device,
        const create_flags flags) {
    static const create_flags VALID_INPUT_FLAGS // Flags directly copied from user input.
        = sphere_data_set_base::property_structured_resource;
    D3D11_BUFFER_DESC bufferDesc;
    auto avgSphereSize = std::abs(this->_sphere_size[1] - this->_sphere_size[0])
        * 0.5f + (std::min)(this->_sphere_size[0], this->_sphere_size[1]);
    auto domainSize = this->extents();
    D3D11_SUBRESOURCE_DATA id;
    std::uniform_real_distribution<float> posDist(0, 1);
    std::uniform_real_distribution<float> radDist(this->_sphere_size[0],
        this->_sphere_size[1]);
    std::vector<std::uint8_t> particles;
    std::mt19937 prng;
    auto stride = random_sphere_data_set::get_stride(this->_type);

    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device to create the vertex "
            "buffer on must not be nullptr.");
    }

    this->_properties = random_sphere_data_set::get_properties(this->_type);
    this->_properties |= (flags & VALID_INPUT_FLAGS);
    this->_max_radius = std::numeric_limits<decltype(this->_max_radius)>::lowest();

    prng.seed(this->_seed);
    particles.resize(this->_size * stride);

    log::instance().write_line(log_level::verbose, "Creating %u random "
        "sphere(s) of type %u on a domain of [%f, %f, %f] with a uniformly "
        "distributed size in [%f, %f]. The random seed is %u.", this->_size,
        static_cast<std::uint32_t>(this->_type), domainSize[0], domainSize[1],
        domainSize[2], this->_sphere_size[0], this->_sphere_size[1],
        this->_seed);
    for (std::uint32_t i = 0; i < this->size(); ++i) {
        auto p = particles.data() + (i * stride);
        auto g = static_cast<float>(i) / static_cast<float>(this->size());
        auto cur = reinterpret_cast<float *>(p);

        cur[0] = posDist(prng) * domainSize[0] - 0.5f * domainSize[0];
        cur[1] = posDist(prng) * domainSize[1] - 0.5f * domainSize[1];
        cur[2] = posDist(prng) * domainSize[2] - 0.5f * domainSize[2];
        cur += 3;

#if 0
        cur->x = cur->y = cur->z = g * domainSize[0] - 0.5f * domainSize[0];
#endif

        switch (this->_type) {
            case sphere_type::pos_rad_intensity:
            case sphere_type::pos_rad_rgba32:
            case sphere_type::pos_rad_rgba8:
                *cur = radDist(prng);
                if (this->_max_radius < *cur) {
                    this->_max_radius = *cur;
                }
                ++cur;
                break;

            default:
                this->_max_radius = avgSphereSize;
        }

        switch (this->_type) {
            case sphere_type::pos_intensity:
            case sphere_type::pos_rad_intensity:
                *reinterpret_cast<float *>(cur) = g;
                break;

            case sphere_type::pos_rgba32:
            case sphere_type::pos_rad_rgba32:
                cur[0] = g;
                cur[1] = g;
                cur[2] = g;
                cur[3] = 1.0f;
                break;

            case sphere_type::pos_rgba8:
            case sphere_type::pos_rad_rgba8: {
                auto s = static_cast<std::uint8_t>(g * 255);
                auto d = reinterpret_cast<std::uint8_t *>(cur);
                d[0] = d[1] = d[2] = s;
                d[3] = 255;
                } break;

            default:
                throw std::runtime_error("Unexpected sphere format.");
        }
    }

    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = static_cast<UINT>(particles.size());
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.CPUAccessFlags = 0;
    if ((flags & property_structured_resource) != 0) {
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.StructureByteStride = stride;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    } else {
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    }

    ::ZeroMemory(&id, sizeof(id));
    id.pSysMem = particles.data();

    {
        this->_buffer = nullptr;
        auto hr = device->CreateBuffer(&bufferDesc, &id, &this->_buffer);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    set_debug_object_name(this->_buffer.p, "random_sphere_data_set");
}


/*
 * trrojan::d3d11::random_sphere_data_set::size
 */
trrojan::d3d11::random_sphere_data_set::size_type
trrojan::d3d11::random_sphere_data_set::size(void) const {
    return this->_size;
}


/*
 * trrojan::d3d11::random_sphere_data_set::stride
 */
trrojan::d3d11::random_sphere_data_set::size_type
trrojan::d3d11::random_sphere_data_set::stride(void) const {
    return random_sphere_data_set::get_stride(this->_type);
}


/*
 * trrojan::d3d11::random_sphere_data_set::minmax
 */
void trrojan::d3d11::random_sphere_data_set::minmax(point_type& i, point_type& a,
        const DirectX::XMFLOAT4& v) {
    if (v.x < i[0]) {
        i[0] = v.x;
    }
    if (v.y < i[1]) {
        i[1] = v.y;
    }
    if (v.z < i[2]) {
        i[2] = v.z;
    }

    if (v.x > a[0]) {
        a[0] = v.x;
    }
    if (v.y > a[1]) {
        a[1] = v.y;
    }
    if (v.z > a[2]) {
        a[2] = v.z;
    }
}


/*
 * trrojan::d3d11::random_sphere_data_set::random_sphere_data_set
 */
trrojan::d3d11::random_sphere_data_set::random_sphere_data_set(void)
    : _max_radius(0.0f), _seed(0), _sphere_size({ 0, 0 }), _size(0),
        _type(sphere_type::unspecified) {
    ::memset(this->_bbox, 0, sizeof(this->_bbox));
}
