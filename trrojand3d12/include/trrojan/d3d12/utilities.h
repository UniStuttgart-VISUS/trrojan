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

    //std::vector<D3D12_INPUT_ELEMENT_DESC> create_cube(ID3D12Device *device,
    //    ID3D12Buffer **outVertices, ID3D12Buffer **outIndices,
    //    const float size = 1.0f);

    //ATL::CComPtr<ID3D12SamplerState> create_linear_sampler(
    //    ID3D12Device *device);

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(ID3D12Device *device,
    //    const UINT width, const UINT height, const UINT elementSize);

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(
    //    ID3D12Texture2D *texture);

    /// <summary>
    /// Initialises the given <see cref="D3D12_SHADER_BYTECODE" />.
    /// </summary>
    /// <param name="dst">The object to be initialised.</param>
    /// <param name="byte_code">The shader byte code.</param>
    /// <param name="cnt_byte_code">The length of <see cref="byte_code" />.
    /// </param>
    inline void set_shader(D3D12_SHADER_BYTECODE& dst, const BYTE *byte_code,
            const std::size_t cnt_byte_code) {
        dst.BytecodeLength = static_cast<UINT>(cnt_byte_code);
        dst.pShaderBytecode = byte_code;
    }

    template<size_t N>
    inline void set_shader(D3D12_SHADER_BYTECODE& dst,
            const BYTE(&byte_code)[N]) {
        set_shader(dst, byte_code, N);
    }

    inline void set_shader(D3D12_SHADER_BYTECODE& dst,
            const std::vector<std::uint8_t>& byte_code) {
        set_shader(dst, byte_code.data(), byte_code.size());
    }

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
