// <copyright file="utilities.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d12.h>

#include "trrojan/d3d12/export.h"


namespace trrojan {
namespace d3d12 {

    //ATL::CComPtr<ID3D12Buffer> TRROJAND3D12_API create_buffer(
    //    ID3D12Device *device, const D3D12_USAGE usage,
    //    const D3D12_BIND_FLAG binding, const void *data, const UINT cntData,
    //    const UINT cpuAccess = 0);

    //ATL::CComPtr<ID3D12ComputeShader> create_compute_shader(ID3D12Device *device,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //inline ATL::CComPtr<ID3D12ComputeShader> create_compute_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_compute_shader(device, byteCode.data(), byteCode.size());
    //}

    //std::vector<D3D12_INPUT_ELEMENT_DESC> create_cube(ID3D12Device *device,
    //    ID3D12Buffer **outVertices, ID3D12Buffer **outIndices,
    //    const float size = 1.0f);

    //ATL::CComPtr<ID3D12DomainShader> create_domain_shader(ID3D12Device *device,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12DomainShader> create_domain_shader(
    //        ID3D12Device *device, const BYTE(&byteCode)[N]) {
    //    return create_domain_shader(device, byteCode, N);
    //}

    //inline ATL::CComPtr<ID3D12DomainShader> create_domain_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_domain_shader(device, byteCode.data(), byteCode.size());
    //}

    //inline ATL::CComPtr<ID3D12DomainShader> create_domain_shader(
    //        ID3D12Device *device, std::nullptr_t) {
    //    return nullptr;
    //}

    //ATL::CComPtr<ID3D12Query> create_event_query(ID3D12Device *device);

    //ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //    ID3D12Device *device, const BYTE *byteCode, const size_t cntByteCode);

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //        ID3D12Device *device, const BYTE(&byteCode)[N]) {
    //    return create_geometry_shader(device, byteCode, N);
    //}

    //inline ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_geometry_shader(device, byteCode.data(), byteCode.size());
    //}

    //inline ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //        ID3D12Device *device, std::nullptr_t) {
    //    return nullptr;
    //}

    //ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //    ID3D12Device *device, const std::string& path);

    //ATL::CComPtr<ID3D12GeometryShader> create_geometry_shader(
    //    ID3D12Device *device, const BYTE *byteCode, const size_t cntByteCode,
    //    const D3D12_SO_DECLARATION_ENTRY *soDecls, const size_t cntSoDecls,
    //    const UINT *bufferStrides = nullptr, const size_t cntBufferStrides = 0,
    //    const UINT rasterisedStream = 0);

    //ATL::CComPtr<ID3D12HullShader> create_hull_shader(ID3D12Device *device,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12HullShader> create_hull_shader(
    //        ID3D12Device *device, const BYTE(&byteCode)[N]) {
    //    return create_hull_shader(device, byteCode, N);
    //}

    //inline ATL::CComPtr<ID3D12HullShader> create_hull_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_hull_shader(device, byteCode.data(), byteCode.size());
    //}


    //inline ATL::CComPtr<ID3D12HullShader> create_hull_shader(
    //        ID3D12Device *device, std::nullptr_t) {
    //    return nullptr;
    //}

    //ATL::CComPtr<ID3D12InputLayout> create_input_layout(ID3D12Device *device,
    //    const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //inline ATL::CComPtr<ID3D12InputLayout> create_input_layout(
    //        ID3D12Device *device,
    //        const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements,
    //        const std::vector<std::uint8_t>& byteCode) {
    //    return create_input_layout(device, elements, byteCode.data(),
    //        byteCode.size());
    //}

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12InputLayout> create_input_layout(
    //        ID3D12Device *device,
    //        const std::vector<D3D12_INPUT_ELEMENT_DESC>& elements,
    //        const BYTE(&byteCode)[N]) {
    //    return create_input_layout(device, elements, byteCode, N);
    //}

    //ATL::CComPtr<ID3D12SamplerState> create_linear_sampler(
    //    ID3D12Device *device);

    //ATL::CComPtr<ID3D12Query> create_pipline_stats_query(ID3D12Device *device);

    //ATL::CComPtr<ID3D12PixelShader> create_pixel_shader(ID3D12Device *device,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12PixelShader> create_pixel_shader(
    //        ID3D12Device *device, const BYTE(&byteCode)[N]) {
    //    return create_pixel_shader(device, byteCode, N);
    //}

    //inline ATL::CComPtr<ID3D12PixelShader> create_pixel_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_pixel_shader(device, byteCode.data(), byteCode.size());
    //}

    //inline ATL::CComPtr<ID3D12PixelShader> create_pixel_shader(
    //        ID3D12Device *device, std::nullptr_t) {
    //    return nullptr;
    //}

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(ID3D12Device *device,
    //    const UINT width, const UINT height, const UINT elementSize);

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(
    //    ID3D12Texture2D *texture);

    //ATL::CComPtr<ID3D12VertexShader> create_vertex_shader(ID3D12Device *device,
    //    const BYTE *byteCode, const size_t cntByteCode);

    //template<size_t N>
    //inline ATL::CComPtr<ID3D12VertexShader> create_vertex_shader(
    //        ID3D12Device *device, const BYTE(&byteCode)[N]) {
    //    return create_vertex_shader(device, byteCode, N);
    //}

    //inline ATL::CComPtr<ID3D12VertexShader> create_vertex_shader(
    //        ID3D12Device *device, const std::vector<std::uint8_t>& byteCode) {
    //    return create_vertex_shader(device, byteCode.data(), byteCode.size());
    //}

    //inline ATL::CComPtr<ID3D12VertexShader> create_vertex_shader(
    //        ID3D12Device *device, std::nullptr_t) {
    //    return nullptr;
    //}

    void set_debug_object_name(ID3D12Object *obj, const char *name);

    /// <summary>
    /// Converts a vector of smart pointers into dumb pointers without
    /// increasing the reference count.
    /// </summary>
    template<class T>
    std::vector<T *> unsmart(std::vector<ATL::CComPtr<T>>& input);

    //void wait_for_event_query(ID3D12DeviceContext *ctx,
    //    ID3D12Asynchronous *query);

    //void wait_for_stats_query(D3D12_QUERY_DATA_PIPELINE_STATISTICS& dst,
    //    ID3D12DeviceContext *ctx, ID3D12Asynchronous *query);

} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/utilities.inl"
