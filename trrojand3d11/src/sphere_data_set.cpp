/// <copyright file="sphere_data_set.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_data_set.h"

#include "sphere_techniques.h"


/*
 * trrojan::d3d11::sphere_data_set_base::property_float_colour
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_float_colour
    = SPHERE_INPUT_FLT_COLOUR;


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_colour
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_colour
    = SPHERE_INPUT_PV_COLOUR;


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_intensity
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_intensity
    = SPHERE_INPUT_PP_INTENSITY | SPHERE_INPUT_PV_INTENSITY;


/*
 * trrojan::d3d11::sphere_data_set_base::property_per_sphere_radius
 */
const trrojan::d3d11::sphere_data_set_base::properties_type
trrojan::d3d11::sphere_data_set_base::property_per_sphere_radius
    = SPHERE_INPUT_PV_RADIUS;


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
        const camera& cam) const {
    point_type bbox[2];
    auto& camPos = cam.get_look_from();
    glm::vec3 centre;
    auto diagLen = 0.0f;
    auto size = this->extents();

    this->bounding_box(bbox[0], bbox[1]);

    for (glm::vec3::length_type i = 0; i < size.size(); ++i) {
        centre[i] = size[i] / 2.0f + bbox[0][i];
        diagLen += size[i] * size[i];
    }
    diagLen = std::sqrt(diagLen);

    diagLen += 0.5f * this->max_radius();
    diagLen *= 0.5f;

    auto viewLen = glm::length(centre - camPos);

    //auto nearPlane = viewLen - diagLen;
    //if (nearPlane < 0.01f) {
    //    nearPlane = 0.01f;
    //}
    auto nearPlane = 0.01f;

    auto farPlane = viewLen + diagLen;
    if (farPlane < nearPlane) {
        farPlane = nearPlane + 1.0f;
    }

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
