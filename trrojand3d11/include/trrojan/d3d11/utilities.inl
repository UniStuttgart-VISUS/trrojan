/// <copyright file="utilities.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::d3d11::create_domain_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11DomainShader> trrojan::d3d11::create_domain_shader(
        ID3D11Device *device, const BYTE(&byteCode)[N]) {
    assert(device != nullptr);
    ATL::CComPtr<ID3D11DomainShader> retval;

    auto hr = device->CreateDomainShader(byteCode, N, nullptr, &retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d11::create_geometry_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11GeometryShader> trrojan::d3d11::create_geometry_shader(
        ID3D11Device *device, const BYTE(&byteCode)[N]) {
    assert(device != nullptr);
    ATL::CComPtr<ID3D11GeometryShader> retval;

    auto hr = device->CreateGeometryShader(byteCode, N, nullptr, &retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d11::create_hull_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11HullShader> trrojan::d3d11::create_hull_shader(
        ID3D11Device *device, const BYTE(&byteCode)[N]) {
    assert(device != nullptr);
    ATL::CComPtr<ID3D11HullShader> retval;

    auto hr = device->CreateHullShader(byteCode, N, nullptr, &retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}
