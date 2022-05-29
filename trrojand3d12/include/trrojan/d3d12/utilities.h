// <copyright file="utilities.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d12.h>

#include "trrojan/d3d12/export.h"
#include "trrojan/d3d12/handle.h"


namespace trrojan {
namespace d3d12 {

    //std::vector<D3D12_INPUT_ELEMENT_DESC> create_cube(ID3D12Device *device,
    //    ID3D12Buffer **outVertices, ID3D12Buffer **outIndices,
    //    const float size = 1.0f);

    //ATL::CComPtr<ID3D12SamplerState> create_linear_sampler(
    //    ID3D12Device *device);

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(ID3D12Device *device,
    //    const UINT width, const UINT height, const UINT elementSize);

    /// <summary>
    /// Create a row-major buffer of the given size on the given device.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="size"></param>
    /// <param name="alignment"></param>
    /// <param name="flags"></param>
    /// <param name="heap_type"></param>
    /// <param name="state"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_buffer(ID3D12Device *device,
        const UINT64 size, const UINT64 alignment = 0,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        const D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT,
        const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST);

    handle<> create_event(const bool manual_reset,
        const bool initially_signalled);

    /// <summary>
    /// Creates a 2D texture which is initially in the render target state.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="format"></param>
    /// <param name="flags"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_render_target(ID3D12Device *device,
        const UINT width, const UINT height, const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    /// <summary>
    /// Create a resource on the default heap which is in
    /// <see cref="D3D12_RESOURCE_STATE_COPY_DEST" />.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="desc"></param>
    /// <param name="heap_type"></param>
    /// <param name="state"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_resource(ID3D12Device *device,
        const D3D12_RESOURCE_DESC& desc,
        const D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_DEFAULT,
        const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COPY_DEST);

    /// <summary>
    /// Create an uninitialised 1D texture which is in
    /// <see cref="D3D12_RESOURCE_STATE_COPY_DEST" />.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="width"></param>
    /// <param name="format"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_texture(ID3D12Device *device,
        const UINT64 width, const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    ATL::CComPtr<ID3D12Resource> create_texture(ID3D12Device *device,
        const UINT64 width, const DXGI_FORMAT format,
        ID3D12GraphicsCommandList *cmd_list, const void *data,
        const UINT row_pitch,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    /// <summary>
    /// Create an uninitialised 2D texture which is in
    /// <see cref="D3D12_RESOURCE_STATE_COPY_DEST" />.
    /// </summary>
    /// <param name="device"></param>
    /// <param name="width"></param>
    /// <param name="height"></param>
    /// <param name="format"></param>
    /// <param name="flags"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_texture(ID3D12Device *device,
        const UINT64 width, const UINT height, const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    /// <summary>
    /// Create an upload buffer for the specified subresource of the given
    /// GPU resource.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="first_subresource"></param>
    /// <param name="cnt_subresources"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_upload_buffer(ID3D12Resource *resource,
        const UINT first_subresource = 0, const UINT cnt_subresources = 1);

    /// <summary>
    /// Gets the device on which the given resource has been allocated.
    /// </summary>
    /// <param name="resource"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Device> get_device(ID3D12Resource *resource);

    /// <summary>
    /// Gets a <see cref="D3D12_TEXTURE_COPY_LOCATION" /> for the given
    /// subresource of the given buffer or texture.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="subresource"></param>
    /// <returns></returns>
    D3D12_TEXTURE_COPY_LOCATION get_copy_location(ID3D12Resource *resource,
        const UINT subresource = 0);

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

    /// <summary>
    /// Sets the specified shader code in <paramref name="dst" />.
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="byte_code"></param>
    template<size_t N>
    inline void set_shader(D3D12_SHADER_BYTECODE& dst,
            const BYTE(&byte_code)[N]) {
        set_shader(dst, byte_code, N);
    }

    /// <summary>
    /// Sets the specified shader code in <paramref name="dst" />.
    /// </summary>
    /// <param name="dst"></param>
    /// <param name="byte_code"></param>
    inline void set_shader(D3D12_SHADER_BYTECODE& dst,
            const std::vector<std::uint8_t>& byte_code) {
        set_shader(dst, byte_code.data(), byte_code.size());
    }

    /// <summary>
    /// Applies the given debug name to the given object for use in the graphics
    /// debugger.
    /// </summary>
    /// <param name="obj"></param>
    /// <param name="name"></param>
    void set_debug_object_name(ID3D12Object *obj, const char *name);

    /// <summary>
    /// Copies the given amount of data into <paramref name="resource" />.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="data"></param>
    /// <param name="cnt"></param>
    void stage_data(ID3D12Resource *resource, const void *data,
        const UINT64 cnt);

    /// <summary>
    /// Allow <paramref name="producer" /> to copy data into
    /// <paramref name="resource" />.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="producer"></param>
    void stage_data(ID3D12Resource *resource,
        const std::function<void(void *, const UINT64)>& producer);

    /// <summary>
    /// Copies the given 2D data into <paramref name="resource" />.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="data"></param>
    /// <param name="cnt_cols"></param>
    /// <param name="cnt_rows"></param>
    /// <param name="row_pitch"></param>
    void stage_data(ID3D12Resource *resource, const void *data,
        const UINT64 cnt_cols, const UINT cnt_rows, UINT64 row_pitch = 0);

    /// <summary>
    /// Converts a vector of smart pointers into dumb pointers without
    /// increasing the reference count.
    /// </summary>
    template<class T>
    std::vector<T *> unsmart(std::vector<ATL::CComPtr<T>>& input);

    void transition_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *resource, const UINT subresource,
        const D3D12_RESOURCE_STATES state_before,
        const D3D12_RESOURCE_STATES state_after);

    /// <summary>
    /// Update <paramref name="dst" /> with the specified
    /// <paramref name="data" /> using the specified <paramref name="staging" />
    /// buffer and transistion it to <paramref name="state_after" />.
    /// </summary>
    /// <param name="cmd_list"></param>
    /// <param name="dst"></param>
    /// <param name="subresource"></param>
    /// <param name="state_after"></param>
    /// <param name="staging"></param>
    /// <param name="data"></param>
    /// <param name="cnt"></param>
    void update_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *dst, const UINT subresource,
        const D3D12_RESOURCE_STATES state_after, ID3D12Resource *staging,
        const void *data, const UINT64 cnt);

    /// <summary>
    /// Update <paramref name="dst" /> with the specified two-dimensional
    /// <paramref name="data" /> using the specified <paramref name="staging" />
    /// buffer and transistion it to <paramref name="state_after" />.
    /// </summary>
    /// <param name="cmd_list"></param>
    /// <param name="dst"></param>
    /// <param name="subresource"></param>
    /// <param name="state_after"></param>
    /// <param name="staging"></param>
    /// <param name="data"></param>
    /// <param name="cnt_cols"></param>
    /// <param name="cnt_rows"></param>
    /// <param name="row_pitch"></param>
    void update_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *dst, const UINT subresource,
        const D3D12_RESOURCE_STATES state_after, ID3D12Resource *staging,
        const void *data, const UINT64 cnt_cols, const UINT cnt_rows,
        const UINT64 row_pitch = 0);

    /// <summary>
    /// Infinitely wait for the given event to become signalled.
    /// </summary>
    /// <param name="handle"></param>
    void wait_for_event(handle<>& handle);

} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/utilities.inl"
