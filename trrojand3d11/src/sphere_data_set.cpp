// <copyright file="sphere_data_set.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_data_set.h"

#include <mmpld.h>

#include "trrojan/log.h"

#include "sphere_techniques.h"


/*
 * trrojan::d3d11::sphere_data_set_base::property_float_colour
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_float_colour
    = SPHERE_INPUT_FLT_COLOUR;
static_assert(mmpld::particle_properties::float_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_FLT_COLOUR),
    "Constant value SPHERE_INPUT_FLT_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_colour
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_colour
    = SPHERE_INPUT_PV_COLOUR;
static_assert(mmpld::particle_properties::per_particle_colour
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_COLOUR),
    "Constant value SPHERE_INPUT_PV_COLOUR must match MMLPD library.");


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_intensity
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_intensity
    = SPHERE_INPUT_PP_INTENSITY | SPHERE_INPUT_PV_INTENSITY;
static_assert(mmpld::particle_properties::per_particle_intensity
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_INTENSITY),
    "Constant value SPHERE_INPUT_PV_INTENSITY must match MMLPD library.");


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_radius
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_radius
    = SPHERE_INPUT_PV_RADIUS;
static_assert(mmpld::particle_properties::per_particle_radius
    == static_cast<mmpld::particle_properties>(SPHERE_INPUT_PV_RADIUS),
    "Constant value SPHERE_INPUT_PV_RADIUS must match MMLPD library.");


/*
 * trrojan::d3d11::sphere_data_set_base::property_structured_resource
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_structured_resource
    = SPHERE_TECHNIQUE_USE_SRV;


/*
 * trrojan::d3d11::sphere_data_set_base::apply
 */
void trrojan::d3d11::sphere_data_set_base::apply(rendering_technique& technique,
        const rendering_technique::shader_stages stages, const UINT idxVb,
        const UINT idxSrv) {
    if ((this->properties() & property_structured_resource) != 0) {
        // Add the data set as structured resource view.
        ATL::CComPtr<ID3D11Device> device;
        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        rendering_technique::srv_type srv;

        this->buffer()->GetDevice(&device);

        ::ZeroMemory(&desc, sizeof(desc));
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = this->size();

        auto hr = device->CreateShaderResourceView(this->buffer().p, &desc,
            &srv);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }

        technique.set_shader_resource_views(srv, stages, idxSrv);

    } else {
        // Add the data set as vertex buffer.
        rendering_technique::vertex_buffer vb;
        vb.buffer = this->buffer().p;
        vb.offset = 0;
        vb.size = this->size();
        vb.stride = this->stride();

        technique.set_vertex_buffers(vb, idxVb);
    }
}


/*
 * trrojan::d3d11::sphere_data_set_base::centre
 */
trrojan::d3d11::sphere_data_set_base::point_type
trrojan::d3d11::sphere_data_set_base::centre(void) const {
    point_type bbox[2];
    point_type retval;

    this->bounding_box(bbox[0], bbox[1]);

    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] = std::abs(bbox[1][i] - bbox[0][i]);
        retval[i] /= 2.0f;
        retval[i] += std::min(bbox[0][i], bbox[1][i]);
    }

    return retval;
}


/*
 * trrojan::d3d11::sphere_data_set_base::clipping_planess
 */
std::pair<float, float> trrojan::d3d11::sphere_data_set_base::clipping_planes(
        const camera& cam, const float globalRadius) const {
    const auto& camPos = cam.get_look_from();
    const auto radius = ((this->_properties & property_per_sphere_radius) != 0)
        ? this->max_radius()
        : globalRadius;
    const auto& view = glm::normalize(cam.get_look_to() - camPos);

    point_type bbox[2];
    auto farPlane = std::numeric_limits<float>::lowest();
    auto nearPlane = (std::numeric_limits<float>::max)();

    this->bounding_box(bbox[0], bbox[1]);

    for (auto x = 0; x < 2; ++x) {
        for (auto y = 0; y < 2; ++y) {
            for (auto z = 0; z < 2; ++z) {
                auto pt = glm::vec3(bbox[x][0], bbox[y][1], bbox[z][2]);
                auto ray = pt - camPos;
                auto dist = glm::dot(view, ray);
                if (dist < nearPlane) nearPlane = dist;
                if (dist > farPlane) farPlane = dist;
            }
        }
    }

    nearPlane -= radius;
    farPlane += radius;

    if (nearPlane < 0.0f) {
        // Plane could become negative in data set, which is illegal. A range of
        // 10k seems to be something our shaders can still handle.
        nearPlane = farPlane / 10000.0f;
    }
    //nearPlane = 0.01f;
    //farPlane *= 1.1f;

    log::instance().write_line(log_level::debug, "Dynamic clipping planes are "
        "located at %f and %f.", nearPlane, farPlane);
    return std::make_pair(nearPlane, farPlane);
}


/*
 * trrojan::d3d11::sphere_data_set_base::extents
 */
std::array<float, 3> trrojan::d3d11::sphere_data_set_base::extents(void) const {
    point_type bbox[2];
    decltype(this->extents()) retval;

    this->bounding_box(bbox[0], bbox[1]);
    for (size_t i = 0; i < retval.size(); ++i) {
        retval[i] = std::abs(bbox[1][i] - bbox[0][i]);
    }

    return retval;
}


/*
 * trrojan::d3d11::sphere_data_set_base::release
 */
void trrojan::d3d11::sphere_data_set_base::release(void) {
    this->_buffer = nullptr;
    this->_properties = 0;
}
