/// <copyright file="utilities.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "trrojan/utilities.h"

#include <atlbase.h>
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

#include "trrojan/d3d11/export.h"

namespace trrojan {
namespace d3d11 {

    HANDLE TRROJAND3D11_API get_shared_handle(ID3D11Resource *resource);

    ATL::CComPtr<ID3D11Buffer> TRROJAND3D11_API create_buffer(
        ID3D11Device *device, const D3D11_USAGE usage,
        const D3D11_BIND_FLAG binding, const void *data, const UINT cntData,
        const UINT cpuAccess = 0);

    /// <summary>
    /// Create a DXGI surface that can be used as Direct2D render target with
    /// the same size as the given swap chain.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="swap_chain"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D11Texture2D> create_compatible_surface(
        ID3D11Device *device, IDXGISwapChain *swap_chain);

    ATL::CComPtr<ID3D11ComputeShader> create_compute_shader(ID3D11Device *device,
        const BYTE *byteCode, const size_t cntByteCode);

    inline ATL::CComPtr<ID3D11ComputeShader> create_compute_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_compute_shader(device, byteCode.data(), byteCode.size());
    }

    /// <summary>
    /// Creates a vertex and an index buffer for a cube of
    /// <see cref="DirectX::XMFLOAT3" /> vertices.
    /// </summary>
    /// <remarks>
    /// </remarks>
    std::vector<D3D11_INPUT_ELEMENT_DESC> create_cube(ID3D11Device *device,
        ID3D11Buffer **outVertices, ID3D11Buffer **outIndices,
        const float size = 1.0f);

    ATL::CComPtr<ID3D11DomainShader> create_domain_shader(ID3D11Device *device,
        const BYTE *byteCode, const size_t cntByteCode);

    template<size_t N>
    inline ATL::CComPtr<ID3D11DomainShader> create_domain_shader(
            ID3D11Device *device, const BYTE(&byteCode)[N]) {
        return create_domain_shader(device, byteCode, N);
    }

    inline ATL::CComPtr<ID3D11DomainShader> create_domain_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_domain_shader(device, byteCode.data(), byteCode.size());
    }

    inline ATL::CComPtr<ID3D11DomainShader> create_domain_shader(
            ID3D11Device *device, std::nullptr_t) {
        return nullptr;
    }

    ATL::CComPtr<ID3D11Query> create_event_query(ID3D11Device *device);

    ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
        ID3D11Device *device, const BYTE *byteCode, const size_t cntByteCode);

    template<size_t N>
    inline ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
            ID3D11Device *device, const BYTE(&byteCode)[N]) {
        return create_geometry_shader(device, byteCode, N);
    }

    inline ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_geometry_shader(device, byteCode.data(), byteCode.size());
    }

    inline ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
            ID3D11Device *device, std::nullptr_t) {
        return nullptr;
    }

    /// <summary>
    /// Create a new geometry shader from the byte code in a shader object file.
    /// </summary>
    ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
        ID3D11Device *device, const std::string& path);

    /// <summary>
    /// Creates a new geometry shader with stream output.
    /// </summary>
    ATL::CComPtr<ID3D11GeometryShader> create_geometry_shader(
        ID3D11Device *device, const BYTE *byteCode, const size_t cntByteCode,
        const D3D11_SO_DECLARATION_ENTRY *soDecls, const size_t cntSoDecls,
        const UINT *bufferStrides = nullptr, const size_t cntBufferStrides = 0,
        const UINT rasterisedStream = 0);

    ATL::CComPtr<ID3D11HullShader> create_hull_shader(ID3D11Device *device,
        const BYTE *byteCode, const size_t cntByteCode);

    template<size_t N>
    inline ATL::CComPtr<ID3D11HullShader> create_hull_shader(
            ID3D11Device *device, const BYTE(&byteCode)[N]) {
        return create_hull_shader(device, byteCode, N);
    }

    inline ATL::CComPtr<ID3D11HullShader> create_hull_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_hull_shader(device, byteCode.data(), byteCode.size());
    }


    inline ATL::CComPtr<ID3D11HullShader> create_hull_shader(
            ID3D11Device *device, std::nullptr_t) {
        return nullptr;
    }

    ATL::CComPtr<ID3D11InputLayout> create_input_layout(ID3D11Device *device,
        const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
        const BYTE *byteCode, const size_t cntByteCode);

    inline ATL::CComPtr<ID3D11InputLayout> create_input_layout(
            ID3D11Device *device,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
            const std::vector<std::uint8_t>& byteCode) {
        return create_input_layout(device, elements, byteCode.data(),
            byteCode.size());
    }

    template<size_t N>
    inline ATL::CComPtr<ID3D11InputLayout> create_input_layout(
            ID3D11Device *device,
            const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements,
            const BYTE(&byteCode)[N]) {
        return create_input_layout(device, elements, byteCode, N);
    }

    ATL::CComPtr<ID3D11SamplerState> create_linear_sampler(
        ID3D11Device *device);

    ATL::CComPtr<ID3D11Query> create_pipline_stats_query(ID3D11Device *device);

    ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(ID3D11Device *device,
        const BYTE *byteCode, const size_t cntByteCode);

    template<size_t N>
    inline ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(
            ID3D11Device *device, const BYTE(&byteCode)[N]) {
        return create_pixel_shader(device, byteCode, N);
    }

    inline ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_pixel_shader(device, byteCode.data(), byteCode.size());
    }

    inline ATL::CComPtr<ID3D11PixelShader> create_pixel_shader(
            ID3D11Device *device, std::nullptr_t) {
        return nullptr;
    }

    /// <summary>
    /// Creates an unordered access view for a structured resource buffer of
    /// the specified element size.
    /// </summary>
    ATL::CComPtr<ID3D11UnorderedAccessView> create_uav(ID3D11Device *device,
        const UINT width, const UINT height, const UINT elementSize);

    /// <summary>
    /// Creates an unordered access view for the given texture.
    /// </summary>
    ATL::CComPtr<ID3D11UnorderedAccessView> create_uav(
        ID3D11Texture2D *texture);

    /// <summary>
    /// Create a vertex shader from the given byte code.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="byteCode"></param>
    /// <param name="cntByteCode"></param>
    /// <returns></returns>
    /// <exception cref="ATL::CAtlException">In case the shader creation failed.
    /// </exception>
    ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(ID3D11Device *device,
        const BYTE *byteCode, const size_t cntByteCode);

    /// <summary>
    /// Create a vertex shader from the given byte code.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="byteCode"></param>
    /// <tparam name="N"></tparam>
    /// <returns></returns>
    /// <exception cref="ATL::CAtlException">In case the shader creation failed.
    /// </exception>
    template<size_t N>
    inline ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(
            ID3D11Device *device, const BYTE(&byteCode)[N]) {
        return create_vertex_shader(device, byteCode, N);
    }

    /// <summary>
    /// Create a vertex shader from the given byte code.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="byteCode"></param>
    /// <returns></returns>
    /// <exception cref="ATL::CAtlException">In case the shader creation failed.
    /// </exception>
    inline ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(
            ID3D11Device *device, const std::vector<std::uint8_t>& byteCode) {
        return create_vertex_shader(device, byteCode.data(), byteCode.size());
    }

    /// <summary>
    /// Create no vertex shader.
    /// </summary>
    /// <param name="device"></param>
    /// <param name=""></param>
    /// <returns></returns>
    /// <exception cref="ATL::CAtlException">In case the shader creation failed.
    /// </exception>
    inline ATL::CComPtr<ID3D11VertexShader> create_vertex_shader(
            ID3D11Device *device, std::nullptr_t) {
        return nullptr;
    }

    /// <summary>
    /// Creates a 1D texture with the Viridis colour map in it.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="outOptSrv">If not <c>nullptr</c>, create a shader resource
    /// view for the texture and return it to this variable.</param>
    /// <returns></returns>
    /// <exception cref="ATL::CAtlException">In case the texture could not be
    /// created.</exception>
    ATL::CComPtr<ID3D11Texture1D> create_viridis_colour_map(
        ID3D11Device *device, ID3D11ShaderResourceView **outOptSrv = nullptr);

    /// <summary>
    /// Gets the back buffer texture of the given swap chain.
    /// </summary>
    /// <param name="swap_chain"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D11Texture2D> get_back_buffer(IDXGISwapChain *swap_chain);

    /// <summary>
    /// Gets the device the texture is resident on.
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D11Device> get_device(ID3D11Texture2D *texture);

    /// <summary>
    /// Gets the underlying DXGI device for a given D3D device.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    ATL::CComPtr<IDXGIDevice> get_device(ID3D11Device *device);

    /// <summary>
    /// Gets the underlying DXGI resource for a given texture.
    /// </summary>
    /// <param name="texture"></param>
    /// <returns></returns>
    ATL::CComPtr<IDXGISurface> get_surface(ID3D11Texture2D *texture);

    /// <summary>
    /// Sets the <c>WKPDID_D3DDebugObjectName</c> of the given object.
    /// </summary>
    template<class T> void set_debug_object_name(T *obj, const char *name);

    /// <summary>
    /// Determines whether the debug layers are installed on Windows 10.
    /// </summary>
    /// <remarks>
    /// If not available, the debug layers can be installed using
    /// <pre>
    /// Dism /online /add-capability /capabilityname:Tools.Graphics.DirectX~~~~0.0.1.0
    /// </pre>
    /// in an elevated command prompt.
    /// </remarks>
    inline bool supports_debug_layer(void) {
        auto hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_NULL, 0,
            D3D11_CREATE_DEVICE_DEBUG, nullptr, 0, D3D11_SDK_VERSION, nullptr,
            nullptr, nullptr);
        return SUCCEEDED(hr);
    }

    /// <summary>
    /// Block the CPU until the given event occurred on the GPU.
    /// </summary>
    void wait_for_event_query(ID3D11DeviceContext *ctx,
        ID3D11Asynchronous *query);

    /// <summary>
    /// Block the CPU until the results of the given event query are available.
    /// </summary>
    void wait_for_stats_query(D3D11_QUERY_DATA_PIPELINE_STATISTICS& dst,
        ID3D11DeviceContext *ctx, ID3D11Asynchronous *query);

} /* end namespace d3d11 */
} /* end namespace trrojan */

#include "trrojan/d3d11/utilities.inl"
