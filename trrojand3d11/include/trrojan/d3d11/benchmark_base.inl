/// <copyright file="benchmark_base.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::d3d11::benchmark_base::create_domain_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11DomainShader>
trrojan::d3d11::benchmark_base::create_domain_shader(d3d11::device& device,
        const BYTE(& byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11DomainShader> retval;

    auto hr = device.d3d_device()->CreateDomainShader(byteCode, N, nullptr,
        &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create domain shader with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}


/*
 * trrojan::d3d11::benchmark_base::create_geometry_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11GeometryShader>
trrojan::d3d11::benchmark_base::create_geometry_shader(d3d11::device& device,
        const BYTE(& byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11GeometryShader> retval;

    auto hr = device.d3d_device()->CreateGeometryShader(byteCode, N, nullptr,
        &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create geometry shader with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}


/*
 * trrojan::d3d11::benchmark_base::create_hull_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11HullShader>
trrojan::d3d11::benchmark_base::create_hull_shader(d3d11::device& device,
        const BYTE(& byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11HullShader> retval;

    auto hr = device.d3d_device()->CreateHullShader(byteCode, N, nullptr,
        &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create hull shader with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}


/*
 * trrojan::d3d11::benchmark_base::create_input_layout
 */
template<size_t N>
ATL::CComPtr<ID3D11InputLayout> 
trrojan::d3d11::benchmark_base::create_input_layout(d3d11::device& device,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
        const BYTE(&byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11InputLayout> retval;

    auto hr = device.d3d_device()->CreateInputLayout(elements.data(),
        static_cast<UINT>(elements.size()), byteCode, N, &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create input layout with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}


/*
 * trrojan::d3d11::benchmark_base::create_pixel_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11PixelShader>
trrojan::d3d11::benchmark_base::create_pixel_shader(d3d11::device& device,
        const BYTE(& byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11PixelShader> retval;

    auto hr = device.d3d_device()->CreatePixelShader(byteCode, N, nullptr,
        &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create pixel shader with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}



/*
 * trrojan::d3d11::benchmark_base::create_vertex_shader
 */
template<size_t N>
ATL::CComPtr<ID3D11VertexShader>
trrojan::d3d11::benchmark_base::create_vertex_shader(d3d11::device& device,
        const BYTE(& byteCode)[N]) {
    assert(device.d3d_device() != nullptr);
    ATL::CComPtr<ID3D11VertexShader> retval;

    auto hr = device.d3d_device()->CreateVertexShader(byteCode, N, nullptr,
        &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create vertex shader with error " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
