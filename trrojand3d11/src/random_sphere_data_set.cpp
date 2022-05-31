// <copyright file="random_sphere_data_set.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
    auto desc = random_sphere_generator::parse_description(configuration);
    return random_sphere_data_set::create(device, flags, desc.sphere_type,
        desc.number, desc.domain_size, desc.sphere_size, desc.seed);
}


/*
 * trrojan::d3d11::random_sphere_data_set::get_input_layout
 */
std::vector<D3D11_INPUT_ELEMENT_DESC>
trrojan::d3d11::random_sphere_data_set::get_input_layout(
        const sphere_type type) {
    return random_sphere_generator::get_input_layout<
        D3D11_INPUT_ELEMENT_DESC>(type);
}


/*
 * trrojan::d3d11::random_sphere_data_set::get_properties
 */
trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::random_sphere_data_set::get_properties(const sphere_type type) {
    properties_type retval = 0;

    switch (type) {
        case sphere_type::pos_rad_intensity:
        case sphere_type::pos_rad_rgba32:
        case sphere_type::pos_rad_rgba8:
            retval |= property_per_sphere_radius;
            break;
    }

    switch (type) {
        case sphere_type::pos_rgba32:
        case sphere_type::pos_rad_rgba32:
            retval |= property_float_colour;
            /* falls through. */
        case sphere_type::pos_rgba8:
        case sphere_type::pos_rad_rgba8:
            retval |= property_per_sphere_colour;
            break;
    }

    switch (type) {
        case sphere_type::pos_intensity:
        case sphere_type::pos_rad_intensity:
            retval |= property_per_sphere_intensity;
            break;
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
    D3D11_SUBRESOURCE_DATA id;
    random_sphere_generator::description particleDesc;

    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device to create the vertex "
            "buffer on must not be nullptr.");
    }

    this->_properties = random_sphere_data_set::get_properties(this->_type);
    this->_properties |= (flags & VALID_INPUT_FLAGS);

    particleDesc.domain_size = this->extents();
    particleDesc.flags = static_cast<random_sphere_generator::create_flags>(
        flags);
    particleDesc.number = this->size();
    particleDesc.seed = this->_seed;
    particleDesc.sphere_size = this->_sphere_size;
    particleDesc.sphere_type = this->_type;

    auto particles = random_sphere_generator::create(this->_max_radius,
        particleDesc);

    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = static_cast<UINT>(particles.size());
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.CPUAccessFlags = 0;
    if ((flags & property_structured_resource) != 0) {
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.StructureByteStride = get_stride(particleDesc.sphere_type);
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
