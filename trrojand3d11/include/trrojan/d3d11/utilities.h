/// <copyright file="utilities.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d11.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    HANDLE TRROJAND3D11_API get_shared_handle(ID3D11Resource *resource);

    ATL::CComPtr<ID3D11Buffer> TRROJAND3D11_API create_buffer(
        ID3D11Device *device, const D3D11_USAGE usage,
        const D3D11_BIND_FLAG binding, const void *data, const UINT cntData,
        const UINT cpuAccess = 0);

    template<size_t N>
    ATL::CComPtr<ID3D11DomainShader> create_domain_shader(ID3D11Device *device,
        const BYTE(&byteCode)[N]);

    template<size_t N>
    ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
        ID3D11Device *device, const BYTE(&byteCode)[N]);

    template<size_t N>
    ATL::CComPtr<ID3D11HullShader> create_hull_shader(
        ID3D11Device *device, const BYTE(&byteCode)[N]);

    template<size_t N>
    ATL::CComPtr<ID3D11InputLayout> create_input_layout(ID3D11Device *device,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
        const BYTE(&byteCode)[N]);

    template<size_t N>
    ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(ID3D11Device *device,
        const BYTE(&byteCode)[N]);

    template<size_t N>
    ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(ID3D11Device *device,
        const BYTE(& byteCode)[N]);

} /* end namespace d3d11 */
} /* end namespace trrojan */

#include "trrojan/d3d11/utilities.inl"
