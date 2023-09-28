// <copyright file="utilities.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <cinttypes>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d12.h>

#include "trrojan/utilities.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/export.h"
#include "trrojan/d3d12/d3dx12.h"
#include "trrojan/d3d12/device.h"
#include "trrojan/d3d12/handle.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Resolve the inner type of a
    /// <see cref="CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT" />.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    template<class TType> struct subobject_inner_type { };

    /// <summary>
    /// Specialisation for actual
    /// <see cref="CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT" />s.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    /// <typeparam name="Type"></typeparam>
    /// <typeparam name="TDefault"></typeparam>
    template<class TType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type,
        class TDefault>
    struct subobject_inner_type<CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<
            TType, Type, TDefault>> {
        typedef TType type;
    };

    /// <summary>
    /// Resolve the type of a
    /// <see cref="CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT" />.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    template<class TType> struct subobject_type { };

    /// <summary>
    /// Specialisation for actual
    /// <see cref="CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT" />s.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    /// <typeparam name="Type"></typeparam>
    /// <typeparam name="TDefault"></typeparam>
    template<class TType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type,
            class TDefault>
    struct subobject_type<CD3DX12_PIPELINE_STATE_STREAM_SUBOBJECT<
            TType, Type, TDefault>> {
        static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE value = Type;
    };

    //std::vector<D3D12_INPUT_ELEMENT_DESC> create_cube(ID3D12Device *device,
    //    ID3D12Buffer **outVertices, ID3D12Buffer **outIndices,
    //    const float size = 1.0f);

    //ATL::CComPtr<ID3D12SamplerState> create_linear_sampler(
    //    ID3D12Device *device);

    //ATL::CComPtr<ID3D12UnorderedAccessView> create_uav(ID3D12Device *device,
    //    const UINT width, const UINT height, const UINT elementSize);

    /// <summary>
    /// Computes the correctly aligned size of a constant buffer with the
    /// specified minimum isze.
    /// </summary>
    /// <param name="size"></param>
    /// <returns></returns>
    inline constexpr UINT64 align_constant_buffer_size(const UINT64 size) {
        return (size + D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)
            & ~(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT - 1);
    }

    /// <summary>
    /// Close the given command list.
    /// </summary>
    void close_command_list(ID3D12GraphicsCommandList *cmd_list);

    /// <summary>
    /// Create a row-major committed buffer of the given size on the
    /// given device.
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

    /// <summary>
    /// Create a row-major committed buffer of the given size satisfying the
    /// alignment requirements of a constant buffer.
    /// </summary>
    ATL::CComPtr<ID3D12Resource> create_constant_buffer(ID3D12Device *device,
        const UINT64 size,
        const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
        const D3D12_HEAP_TYPE heap_type = D3D12_HEAP_TYPE_UPLOAD,
        const D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_GENERIC_READ);

    /// <summary>
    /// Creates an event kernel object.
    /// </summary>
    /// <param name="manual_reset"></param>
    /// <param name="initially_signalled"></param>
    /// <returns></returns>
    handle<> create_event(const bool manual_reset,
        const bool initially_signalled);

    /// <summary>
    /// Creates a 2D texture which is initially in the present state.
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
    //ATL::CComPtr<ID3D12Resource> create_texture(ID3D12Device *device,
    //    const UINT64 width, const UINT height, const DXGI_FORMAT format,
    //    const D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);

    /// <summary>
    /// Create an upload buffer for the specified subresource of the given
    /// GPU resource.
    /// </summary>
    /// <remarks>
    /// The function determines the copyable footprints of the given
    /// <paramref name="resource" /> and creates an upload buffer with this
    /// size, thus fulfilling implicitly all alignment requirements for upload
    /// buffers.
    /// </remarks>
    /// <param name="resource"></param>
    /// <param name="first_subresource"></param>
    /// <param name="cnt_subresources"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_upload_buffer(ID3D12Resource *resource,
        const UINT first_subresource = 0, const UINT cnt_subresources = 1);

    /// <summary>
    /// Create an upload buffer of the specified size.
    /// </summary>
    /// <remarks>
    /// The function will make sure that the size of the buffer is aligned to a
    /// 256-byte boundary, so the actual size of the resource returned might be
    /// larger than the one requested.
    /// </remarks>
    /// <param name="device"></param>
    /// <param name="size"></param>
    /// <param name="alignment"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_upload_buffer(ID3D12Device *device,
        const UINT64 size, const UINT64 alignment = 0);

    /// <summary>
    /// Create an upload buffer and fill it with the specified data.
    /// </summary>
    /// <remarks>
    /// The function will make sure that the size of the buffer is aligned to a
    /// 256-byte boundary, so the actual size of the resource returned might be
    /// larger than the one requested. However, at most <paramref name="size" />
    /// bytes will be copied to the begin of the buffer.
    /// </remarks>
    /// <param name="device"></param>
    /// <param name="data"></param>
    /// <param name="size"></param>
    /// <param name="alignment"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_upload_buffer(ID3D12Device *device,
        const void *data, const UINT64 size, const UINT64 alignment = 0);

    /// <summary>
    /// Create a 1D upload buffer containing the Viridis colour map.
    /// </summary>
    /// <param name="device"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_viridis_colour_map(
        ID3D12Device *device);

    /// <summary>
    /// Create a 1D GPU-only texture containing the Viridis colour map and
    /// transition it to the requested <paramref name="state" />.
    /// </summary>
    /// <param name="device">The device to create the colour map on. The device
    /// must have a valid graphics command queue to perform the on-device copy
    /// and transitioning of the resource.</param>
    /// <param name="cmd_list">A command list used to perform the transition.
    /// The command list must be in a recordable state and will be closed and
    /// executed by the function.</param>
    /// <param name="state">The state to transition the resource to. This
    /// parameter defaults to<
    /// <c>D3D12_RESOURCE_STATE_ALL_SHADER_RESOURCE</c>.</param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Resource> create_viridis_colour_map(device& device,
        ID3D12GraphicsCommandList *cmd_list, const D3D12_RESOURCE_STATES state
        = static_cast<D3D12_RESOURCE_STATES>(0x40 | 0x80));

    /// <summary>
    /// Gets the device the child belongs to.
    /// </summary>
    /// <param name="child"></param>
    /// <returns></returns>
    ATL::CComPtr<ID3D12Device> get_device(ID3D12DeviceChild *child);

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
    /// Index into an array aligned according to constant buffer alignment
    /// rules.
    /// </summary>
    /// <typeparam name="TPointer"></typeparam>
    /// <param name="ptr"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    template<class TPointer>
    inline TPointer *index_constant_buffer(TPointer *ptr, const std::size_t n) {
        auto p = reinterpret_cast<std::uint8_t *>(ptr);
        p += n * align_constant_buffer_size(sizeof(TPointer));
        return reinterpret_cast<TPointer *>(p);
    }

    /// <summary>
    /// Offsets the given pointer by the given number of bytes.
    /// </summary>
    /// <typeparam name="TPointer"></typeparam>
    /// <param name="ptr"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    template<class TPointer>
    inline TPointer *offset_by(TPointer *ptr, const std::size_t offset) {
        typedef std::conditional<std::is_const<TPointer>::value,
            const std::uint8_t *, std::uint8_t *>::type arithmetic_type;
        auto retval = reinterpret_cast<arithmetic_type>(ptr) + offset;
        return reinterpret_cast<TPointer *>(retval);
    }

    /// <summary>
    /// Offsets the given virtual address by the given number of bytes.
    /// </summary>
    /// <param name="address"></param>
    /// <param name="offset"></param>
    /// <returns></returns>
    inline D3D12_GPU_VIRTUAL_ADDRESS offset_by(
            const D3D12_GPU_VIRTUAL_ADDRESS address,
            const std::size_t offset) {
        return (address + offset);
    }

    /// <summary>
    /// Offsets the given pointer by the size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <typeparam name="TPointer"></typeparam>
    /// <param name="ptr"></param>
    /// <returns></returns>
    template<class TOffset, class TPointer>
    inline TPointer *offset_by(TPointer *ptr) {
        return offset_by(ptr, sizeof(TOffset));
    }

    /// <summary>
    /// Offsets the given pointer by a multiple of the size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <typeparam name="TPointer"></typeparam>
    /// <param name="ptr"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    template<class TOffset, class TPointer>
    inline TPointer *offset_by_n(TPointer *ptr, const std::size_t n) {
        return offset_by(ptr, n * sizeof(TOffset));
    }

    /// <summary>
    /// Offsets the given pointer by a multiple of the aligned size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    /// <typeparam name="TPointer"></typeparam>
    /// <param name="ptr"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    template<class TOffset, std::size_t Alignment, class TPointer>
    inline TPointer *offset_by_n(TPointer *ptr, const std::size_t n) {
        auto aligned_size = (sizeof(TOffset) + Alignment) & ~(Alignment - 1);
        return offset_by(ptr, n * aligned_size);
    }

    /// <summary>
    /// Offsets the given virtual address by the size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <param name="address"></param>
    /// <returns></returns>
    template<class TOffset>
    inline D3D12_GPU_VIRTUAL_ADDRESS offset_by(
            const D3D12_GPU_VIRTUAL_ADDRESS address) {
        return offset_by(address, sizeof(TOffset));
    }

    /// <summary>
    /// Offsets the given virtual address by a mupltiple of the size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <param name="address"></param>
    /// <returns></returns>
    template<class TOffset>
    inline D3D12_GPU_VIRTUAL_ADDRESS offset_by_n(
            const D3D12_GPU_VIRTUAL_ADDRESS address, const std::size_t n) {
        return offset_by(address, n * sizeof(TOffset));
    }

    /// <summary>
    /// Offsets the given virtual address by a multiple of the aligned size of
    /// <typeparamref name="TOffset" />.
    /// </summary>
    /// <typeparam name="TOffset"></typeparam>
    /// <typeparam name="Alignment"></typeparam>
    /// <param name="ptr"></param>
    /// <param name="n"></param>
    /// <returns></returns>
    template<class TOffset, std::size_t Alignment>
    inline D3D12_GPU_VIRTUAL_ADDRESS offset_by_n(
            D3D12_GPU_VIRTUAL_ADDRESS address, const std::size_t n) {
        auto horst = sizeof(TOffset);
        auto aligned_size = (sizeof(TOffset) + Alignment) & ~(Alignment - 1);
        return offset_by(address, n * aligned_size);
    }

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
    /// Applies the given debug name to the given object for use in the graphics
    /// debugger.
    /// </summary>
    /// <typeparam name="Args"></typeparam>
    /// <param name="obj"></param>
    /// <param name="fmt"></param>
    /// <param name="...args"></param>
    template<class... Args>
    inline void set_debug_object_name(ID3D12Object *obj, const char *fmt,
            Args&&... args) {
        auto name = fmt::format(fmt, std::forward<Args>(args)...);
        set_debug_object_name(obj, name.c_str());
    }

    /// <summary>
    /// Copies the given amount of data into <paramref name="resource" />, which
    /// must be located on an upload heap.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="data"></param>
    /// <param name="cnt"></param>
    void stage_data(ID3D12Resource *resource, const void *data,
        const UINT64 cnt);

    /// <summary>
    /// Allow <paramref name="producer" /> to copy data into
    /// <paramref name="resource" />, which must be located on an upload heap.
    /// </summary>
    /// <param name="resource"></param>
    /// <param name="producer"></param>
    void stage_data(ID3D12Resource *resource,
        const std::function<void(void *, const UINT64)>& producer);

    /// <summary>
    /// Copies the given 2D data into <paramref name="resource" />, which must
    /// be located on an upload heap.
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

    void transition_resource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *resource, const D3D12_RESOURCE_STATES state_before,
        const D3D12_RESOURCE_STATES state_after);

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
