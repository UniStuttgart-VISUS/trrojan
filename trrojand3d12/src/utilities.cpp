// <copyright file="utilities.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/utilities.h"

#include <DirectXMath.h>
#include <Psapi.h>

#include "trrojan/com_error_category.h"
#include "trrojan/log.h"
#include "trrojan/io.h"

#include "trrojan/d3d12/graphics_pipeline_builder.h"


///*
// * trrojan::d3d12::create_cube
// */
//std::vector<D3D12_INPUT_ELEMENT_DESC>  trrojan::d3d12::create_cube(
//        ID3D12Device *device, ID3D12Buffer **outVertices,
//        ID3D12Buffer **outIndices, const float size) {
//    assert(device != nullptr);
//    assert(outVertices != nullptr);
//    assert(outIndices != nullptr);
//    assert(*outVertices == nullptr);
//    assert(*outIndices == nullptr);
//
//    static const std::vector<D3D12_INPUT_ELEMENT_DESC> retval = {
//        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_PER_VERTEX_DATA, 0 }
//    };
//
//    static const DirectX::XMFLOAT3 vertices[] = {
//        // front
//        {-0.5f * size, -0.5f * size, 0.5f * size },
//        { 0.5f * size, -0.5f * size, 0.5f * size },
//        { 0.5f * size, 0.5f * size, 0.5f * size },
//        { -0.5f * size, 0.5f * size, 0.5f * size },
//        // back
//        { -0.5f * size, -0.5f * size, -0.5f * size },
//        { 0.5f * size, -0.5f * size, -0.5f * size },
//        { 0.5f * size, 0.5f * size, -0.5f * size },
//        { -0.5f * size, 0.5f * size, -0.5f * size },
//    };
//
//    static const short indices[] = {
//        // front
//        2, 1, 0,
//        0, 3, 2,
//        // right
//        6, 5, 1,
//        1, 2, 6,
//        // back
//        5, 6, 7,
//        7, 4, 5,
//        // left
//        3, 0, 4,
//        4, 7, 3,
//        // bottom
//        1, 5, 4,
//        4, 0, 1,
//        // top
//        6, 2, 3,
//        3, 7, 6
//    };
//
//    D3D12_BUFFER_DESC desc;
//    D3D12_SUBRESOURCE_DATA id;
//
//    {
//        ::ZeroMemory(&desc, sizeof(desc));
//        desc.BindFlags = D3D12_BIND_VERTEX_BUFFER;
//        desc.ByteWidth = sizeof(vertices);
//        desc.Usage = D3D12_USAGE_IMMUTABLE;
//
//        ::ZeroMemory(&id, sizeof(id));
//        id.pSysMem = vertices;
//
//        auto hr = device->CreateBuffer(&desc, &id, outVertices);
//        if (FAILED(hr)) {
//            throw std::system_error(hr, com_category());
//        }
//
//        set_debug_object_name(*outVertices, "Cube vertex buffer");
//    }
//
//    {
//        ::ZeroMemory(&desc, sizeof(desc));
//        desc.BindFlags = D3D12_BIND_INDEX_BUFFER;
//        desc.ByteWidth = sizeof(vertices);
//        desc.Usage = D3D12_USAGE_IMMUTABLE;
//
//        ::ZeroMemory(&id, sizeof(id));
//        id.pSysMem = indices;
//
//        auto hr = device->CreateBuffer(&desc, &id, outIndices);
//        if (FAILED(hr)) {
//            throw std::system_error(hr, com_category());
//        }
//
//        set_debug_object_name(*outIndices, "Cube index buffer");
//    }
//
//    return retval;
//}


/// <summary>
/// Check whether <paramref name="ptr" /> is non-null.
/// </summary>
/// <typeparam name="TPointer"></typeparam>
/// <param name="ptr"></param>
template<class TPointer>
static void check_not_null(const TPointer *ptr) {
    if (ptr == nullptr) {
        throw std::system_error(E_POINTER, trrojan::com_category());
    }
}


/// <summary>
/// Check whether <paramref name="resource" /> is a valid staging buffer.
/// </summary>
static void check_staging_buffer(ID3D12Resource *resource) {
    if (resource == nullptr) {
        throw std::invalid_argument("A valid resource is required to stage "
            "data.");
    }

    {
        auto desc = resource->GetDesc();
        if (desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER) {
            throw std::invalid_argument("The staging resource must be a "
                "buffer.");
        }
    }
}


/// <summary>
/// Check whether <paramref name="resource" /> is a valid staging buffer holding
/// at least <paramref name="cnt" /> bytes.
/// </summary>
static void check_staging_buffer(ID3D12Resource *resource, const UINT64 cnt) {
    check_staging_buffer(resource);
    assert(resource != nullptr);

    {
        auto device = trrojan::d3d12::get_device(resource);
        UINT64 size;
        device->GetCopyableFootprints(&resource->GetDesc(), 0, 1, 0, nullptr,
            nullptr, nullptr, &size);
        if (size < cnt) {
            throw std::invalid_argument("The staging buffer is too small for "
                "the amount of data specified.");
        }
    }
}


/*
 * trrojan::d3d12::close_command_list
 */
void trrojan::d3d12::close_command_list(ID3D12GraphicsCommandList *cmd_list) {
    assert(cmd_list != nullptr);
    auto hr = cmd_list->Close();
    if (FAILED(hr)) {
        log::instance().write_line(log_level::error, "Closing command list {:p} "
            "failed with error code 0x{:x}", static_cast<void *>(cmd_list), hr);
        throw std::system_error(hr, com_category());
    }
}


/*
 * trrojan::d3d12::create_buffer
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_buffer(ID3D12Device *device,
        const UINT64 size, const UINT64 alignment,
        const D3D12_RESOURCE_FLAGS flags, const D3D12_HEAP_TYPE heap_type,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);

    D3D12_RESOURCE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Height = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_UNKNOWN;
    desc.Width = size;
    desc.Alignment = alignment;
    desc.Flags = flags;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    return create_resource(device, desc, heap_type, state);
}


/*
 * trrojan::d3d12::create_compute_pipeline
 */
winrt::com_ptr<ID3D12PipelineState> trrojan::d3d12::create_compute_pipeline(
        ID3D12Device *device, const BYTE *shader, const SIZE_T size,
        ID3D12RootSignature *signature) {
    assert(device != nullptr);
    assert(shader != nullptr);
    assert(signature != nullptr);

    winrt::com_ptr<ID3D12PipelineState> retval;

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.pRootSignature = signature;
    desc.CS.pShaderBytecode = shader;
    desc.CS.BytecodeLength = size;

    auto hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::create_compute_pipeline
 */
winrt::com_ptr<ID3D12PipelineState> trrojan::d3d12::create_compute_pipeline(
        winrt::com_ptr<ID3D12RootSignature>& signature, ID3D12Device *device,
        const BYTE *shader, const SIZE_T size) {
    assert(device != nullptr);
    assert(shader != nullptr);
    signature = graphics_pipeline_builder::root_signature_from_shader(device,
        shader, size);
    return create_compute_pipeline(device, shader, size, signature.get());
}


/*
 * trrojan::d3d12::create_constant_buffer
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_constant_buffer(
        ID3D12Device *device, const UINT64 size,
        const D3D12_RESOURCE_FLAGS flags, const D3D12_HEAP_TYPE heap_type,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);
    return create_buffer(device, align_constant_buffer_size(size),
        0, flags, heap_type, state);
}


/*
 * trrojan::d3d12::create_event
 */
trrojan::d3d12::handle<> trrojan::d3d12::create_event(const bool manual_reset,
        const bool initially_signalled) {
    handle<> retval = ::CreateEvent(nullptr, manual_reset, initially_signalled,
        nullptr);
    if (!retval) {
        throw std::system_error(::GetLastError(), std::system_category());
    }
    return retval;
}


/*
 * trrojan::d3d12::create_fence
 */
winrt::com_ptr<ID3D12Fence> trrojan::d3d12::create_fence(ID3D12Device *device,
        const UINT64 initial_value) {
    if (device == nullptr) {
        throw std::system_error(E_POINTER, com_category());
    }

    winrt::com_ptr<ID3D12Fence> retval;
    auto hr = device->CreateFence(initial_value, D3D12_FENCE_FLAG_NONE,
        ::IID_ID3D12Fence, reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::create_file_mapping
 */
winrt::handle trrojan::d3d12::create_file_mapping(winrt::file_handle& handle,
        const DWORD protect, const std::size_t size) {
    ULARGE_INTEGER s;
    s.QuadPart = size;

    winrt::handle retval(::CreateFileMappingW(handle.get(), nullptr, protect,
        s.HighPart, s.LowPart, nullptr));
    if (!retval) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::create_heap
 */
winrt::com_ptr<ID3D12Heap> trrojan::d3d12::create_heap(ID3D12Device *device,
        const D3D12_HEAP_DESC& desc) {
    if (device == nullptr) {
        throw std::system_error(E_POINTER, com_category());
    }

    log::instance().write_line(log_level::debug, "Creating heap of {0} Bytes "
        "with an alignment of {1} ...", desc.SizeInBytes, desc.Alignment);

    winrt::com_ptr<ID3D12Heap> retval;
    auto hr = device->CreateHeap(&desc, IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::create_heap
 */
winrt::com_ptr<ID3D12Heap> trrojan::d3d12::create_heap(ID3D12Device *device,
        const D3D12_RESOURCE_ALLOCATION_INFO& alloc_info,
        const std::size_t cnt,
        const D3D12_HEAP_TYPE type,
        const D3D12_HEAP_FLAGS flags) {
    D3D12_HEAP_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Alignment = alloc_info.Alignment;
    desc.Flags = flags;
    desc.Properties.Type = type;
    desc.SizeInBytes = cnt * alloc_info.SizeInBytes;

    return create_heap(device, desc);
}


/*
 * trrojan::d3d12::create_render_target
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_render_target(
        ID3D12Device *device, const UINT width, const UINT height,
        const DXGI_FORMAT format, const D3D12_RESOURCE_FLAGS flags) {
    assert(device != nullptr);

    D3D12_RESOURCE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = flags | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    return create_resource(device, desc, D3D12_HEAP_TYPE_DEFAULT,
        D3D12_RESOURCE_STATE_PRESENT);
}


/*
 * trrojan::d3d12::create_resource
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_resource(
        ID3D12Device *device, const D3D12_RESOURCE_DESC& desc,
        const D3D12_HEAP_TYPE heap_type,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);

    D3D12_HEAP_PROPERTIES props;
    ::ZeroMemory(&props, sizeof(props));
    props.CreationNodeMask = 0x1;
    props.VisibleNodeMask = 0x1;
    props.Type = heap_type;

    winrt::com_ptr<ID3D12Resource> retval;
    auto hr = device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE,
        &desc, state, nullptr, ::IID_ID3D12Resource,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::create_texture
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_texture(
        ID3D12Device *device,
        const UINT64 width,
        const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);

    D3D12_RESOURCE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = 1;
    desc.Flags = flags;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;

    return create_resource(device, desc, D3D12_HEAP_TYPE_DEFAULT, state);
}


/*
 * trrojan::d3d12::create_texture
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_texture(
        ID3D12Device *device,
        const UINT64 width,
        const UINT height,
        const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);

    D3D12_RESOURCE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = flags;
    desc.DepthOrArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    return create_resource(device, desc, D3D12_HEAP_TYPE_DEFAULT, state);
}


/*
 * trrojan::d3d12::create_texture
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_texture(
        ID3D12Device *device,
        const UINT64 width,
        const UINT height,
        const UINT16 depth,
        const DXGI_FORMAT format,
        const D3D12_RESOURCE_FLAGS flags,
        const D3D12_RESOURCE_STATES state) {
    assert(device != nullptr);

    D3D12_RESOURCE_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.MipLevels = 1;
    desc.Format = format;
    desc.Width = width;
    desc.Height = height;
    desc.Flags = flags;
    desc.DepthOrArraySize = depth;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;

    return create_resource(device, desc, D3D12_HEAP_TYPE_DEFAULT, state);
}


/*
 * trrojan::d3d12::create_upload_buffer
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_upload_buffer(
        ID3D12Device *device, const UINT64 size, const UINT64 alignment) {
    static constexpr UINT64 ALIGNMENT = 255;
    auto aligned_size = (size + ALIGNMENT) & ~ALIGNMENT;
    return create_buffer(device, aligned_size, alignment,
        D3D12_RESOURCE_FLAG_NONE, D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_GENERIC_READ);
}


/*
 * trrojan::d3d12::create_upload_buffer
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_upload_buffer(
        ID3D12Device *device, const void *data, const UINT64 size,
        const UINT64 alignment) {
    auto retval = create_upload_buffer(device, size, alignment);

    if (data != nullptr) {
        void *map;
        D3D12_RANGE nothing = { 0, 0 };

        auto hr = retval->Map(0, &nothing, &map);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }

        ::memcpy(map, data, size);

        retval->Unmap(0, nullptr);
    }

    return retval;
}


/*
 * trrojan::d3d12::create_upload_buffer_for
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_upload_buffer_for(
        ID3D12Resource *resource, const UINT first_subresource,
        const UINT cnt_subresources) {
    assert(resource != nullptr);
    // Cf. https://github.com/microsoft/DirectX-Graphics-Samples/blob/164b072185a5360c43c5f0b64e2f672b7f423f95/Libraries/D3D12RaytracingFallback/Include/d3dx12.h#L1875
    const auto desc = resource->GetDesc();
    auto device = get_device(resource);
    UINT64 size;

    device->GetCopyableFootprints(&desc, first_subresource, cnt_subresources,
        0, nullptr, nullptr, nullptr, &size);

    return create_buffer(device, size, 0, D3D12_RESOURCE_FLAG_NONE,
        D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);
}


/*
 * trrojan::d3d12::create_viridis_colour_map
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_viridis_colour_map(
        ID3D12Device *device) {
    assert(device != nullptr);
#pragma warning(disable: 4244)
#pragma warning(disable: 4838)
    static const BYTE data[] = {
        255 * 0.26700401f, 255 * 0.00487433f, 255 * 0.32941519f, 255,
        255 * 0.26851048f, 255 * 0.00960483f, 255 * 0.33542652f, 255,
        255 * 0.26994384f, 255 * 0.01462494f, 255 * 0.34137895f, 255,
        255 * 0.27130489f, 255 * 0.01994186f, 255 * 0.34726862f, 255,
        255 * 0.27259384f, 255 * 0.02556309f, 255 * 0.35309303f, 255,
        255 * 0.27380934f, 255 * 0.03149748f, 255 * 0.35885256f, 255,
        255 * 0.27495242f, 255 * 0.03775181f, 255 * 0.36454323f, 255,
        255 * 0.27602238f, 255 * 0.04416723f, 255 * 0.37016418f, 255,
        255 * 0.2770184f , 255 * 0.05034437f, 255 * 0.37571452f, 255,
        255 * 0.27794143f, 255 * 0.05632444f, 255 * 0.38129074f, 255,
        255 * 0.27879067f, 255 * 0.06214536f, 255 * 0.38659204f, 255,
        255 * 0.2795655f , 255 * 0.06783587f, 255 * 0.39191723f, 255,
        255 * 0.28026658f, 255 * 0.07341724f, 255 * 0.39716349f, 255,
        255 * 0.28089358f, 255 * 0.07890703f, 255 * 0.40232944f, 255,
        255 * 0.28144581f, 255 * 0.0843197f , 255 * 0.40741404f, 255,
        255 * 0.28192358f, 255 * 0.08966622f, 255 * 0.41241521f, 255,
        255 * 0.28232739f, 255 * 0.09495545f, 255 * 0.41733086f, 255,
        255 * 0.28265633f, 255 * 0.10019576f, 255 * 0.42216032f, 255,
        255 * 0.28291049f, 255 * 0.10539345f, 255 * 0.42690202f, 255,
        255 * 0.28309095f, 255 * 0.12055307f, 255 * 0.43155375f, 255,
        255 * 0.28319704f, 255 * 0.12567966f, 255 * 0.43612482f, 255,
        255 * 0.28322882f, 255 * 0.12077701f, 255 * 0.44058404f, 255,
        255 * 0.28318684f, 255 * 0.12584799f, 255 * 0.44496f   , 255,
        255 * 0.283072f  , 255 * 0.13089477f, 255 * 0.44924127f, 255,
        255 * 0.28288389f, 255 * 0.13592005f, 255 * 0.45342734f, 255,
        255 * 0.28262297f, 255 * 0.14092556f, 255 * 0.45751726f, 255,
        255 * 0.28229037f, 255 * 0.14591233f, 255 * 0.46150995f, 255,
        255 * 0.28188676f, 255 * 0.15088147f, 255 * 0.46540474f, 255,
        255 * 0.28141228f, 255 * 0.15583425f, 255 * 0.46920128f, 255,
        255 * 0.28086773f, 255 * 0.16077132f, 255 * 0.47289909f, 255,
        255 * 0.28025468f, 255 * 0.16569272f, 255 * 0.47649762f, 255,
        255 * 0.27957399f, 255 * 0.17059884f, 255 * 0.47999675f, 255,
        255 * 0.27882618f, 255 * 0.1754902f , 255 * 0.48339654f, 255,
        255 * 0.27801236f, 255 * 0.18036684f, 255 * 0.48669702f, 255,
        255 * 0.27713437f, 255 * 0.18522836f, 255 * 0.48989831f, 255,
        255 * 0.27619376f, 255 * 0.19007447f, 255 * 0.49300074f, 255,
        255 * 0.27519126f, 255 * 0.1949054f , 255 * 0.49600488f, 255,
        255 * 0.27412802f, 255 * 0.19972086f, 255 * 0.49891231f, 255,
        255 * 0.27300596f, 255 * 0.20452049f, 255 * 0.50172076f, 255,
        255 * 0.27182812f, 255 * 0.20930306f, 255 * 0.50443413f, 255,
        255 * 0.27059473f, 255 * 0.21406899f, 255 * 0.50705243f, 255,
        255 * 0.26930756f, 255 * 0.21881782f, 255 * 0.50957678f, 255,
        255 * 0.26796846f, 255 * 0.22354912f, 255 * 0.5120084f , 255,
        255 * 0.26657984f, 255 * 0.2282621f , 255 * 0.5143487f , 255,
        255 * 0.2651445f , 255 * 0.23295593f, 255 * 0.5165993f , 255,
        255 * 0.2636632f , 255 * 0.23763078f, 255 * 0.51876163f, 255,
        255 * 0.26213801f, 255 * 0.24228619f, 255 * 0.52083736f, 255,
        255 * 0.26057103f, 255 * 0.2469217f , 255 * 0.52282822f, 255,
        255 * 0.25896451f, 255 * 0.25153685f, 255 * 0.52473609f, 255,
        255 * 0.25732244f, 255 * 0.2561304f , 255 * 0.52656332f, 255,
        255 * 0.25564519f, 255 * 0.26070284f, 255 * 0.52831252f, 255,
        255 * 0.25393498f, 255 * 0.26525384f, 255 * 0.52998273f, 255,
        255 * 0.25219404f, 255 * 0.26978306f, 255 * 0.53157905f, 255,
        255 * 0.25042462f, 255 * 0.27429024f, 255 * 0.53310261f, 255,
        255 * 0.24862899f, 255 * 0.27877509f, 255 * 0.53455561f, 255,
        255 * 0.2468124f , 255 * 0.28323662f, 255 * 0.53594093f, 255,
        255 * 0.24497208f, 255 * 0.28767547f, 255 * 0.53726018f, 255,
        255 * 0.24312324f, 255 * 0.29209154f, 255 * 0.53851561f, 255,
        255 * 0.24123708f, 255 * 0.29648471f, 255 * 0.53970946f, 255,
        255 * 0.23934575f, 255 * 0.30085494f, 255 * 0.54084398f, 255,
        255 * 0.23744138f, 255 * 0.30520222f, 255 * 0.5419214f , 255,
        255 * 0.23552606f, 255 * 0.30952657f, 255 * 0.54294396f, 255,
        255 * 0.23360277f, 255 * 0.31382773f, 255 * 0.54391424f, 255,
        255 * 0.2316735f , 255 * 0.3181058f , 255 * 0.54483444f, 255,
        255 * 0.22973926f, 255 * 0.32236127f, 255 * 0.54570633f, 255,
        255 * 0.22780192f, 255 * 0.32659432f, 255 * 0.546532f  , 255,
        255 * 0.2258633f , 255 * 0.33080515f, 255 * 0.54731353f, 255,
        255 * 0.22392515f, 255 * 0.334994f  , 255 * 0.54805291f, 255,
        255 * 0.22198915f, 255 * 0.33916124f, 255 * 0.54875212f, 255,
        255 * 0.22005691f, 255 * 0.34330688f, 255 * 0.54941304f, 255,
        255 * 0.21812995f, 255 * 0.34743154f, 255 * 0.55003755f, 255,
        255 * 0.21620971f, 255 * 0.35153548f, 255 * 0.55062743f, 255,
        255 * 0.21429757f, 255 * 0.35561907f, 255 * 0.5512844f , 255,
        255 * 0.21239477f, 255 * 0.35968273f, 255 * 0.55171012f, 255,
        255 * 0.2105031f , 255 * 0.36372671f, 255 * 0.55220646f, 255,
        255 * 0.20862342f, 255 * 0.36775151f, 255 * 0.55267486f, 255,
        255 * 0.20675628f, 255 * 0.37175775f, 255 * 0.55312653f, 255,
        255 * 0.20490257f, 255 * 0.37574589f, 255 * 0.55353282f, 255,
        255 * 0.20306309f, 255 * 0.37971644f, 255 * 0.55392505f, 255,
        255 * 0.20123854f, 255 * 0.38366989f, 255 * 0.55429441f, 255,
        255 * 0.1994295f , 255 * 0.38760678f, 255 * 0.55464205f, 255,
        255 * 0.1976365f , 255 * 0.39152762f, 255 * 0.55496905f, 255,
        255 * 0.19585993f, 255 * 0.39543297f, 255 * 0.55527637f, 255,
        255 * 0.19410009f, 255 * 0.39932336f, 255 * 0.55556494f, 255,
        255 * 0.19235719f, 255 * 0.40319934f, 255 * 0.55583559f, 255,
        255 * 0.19063135f, 255 * 0.40706148f, 255 * 0.55608907f, 255,
        255 * 0.18892259f, 255 * 0.41091033f, 255 * 0.55632606f, 255,
        255 * 0.18723083f, 255 * 0.41474645f, 255 * 0.55654717f, 255,
        255 * 0.18555593f, 255 * 0.4185704f , 255 * 0.55675292f, 255,
        255 * 0.18389763f, 255 * 0.42238275f, 255 * 0.55694377f, 255,
        255 * 0.18225561f, 255 * 0.42618405f, 255 * 0.5571201f , 255,
        255 * 0.18062949f, 255 * 0.42997486f, 255 * 0.55728221f, 255,
        255 * 0.17901879f, 255 * 0.43375572f, 255 * 0.55743035f, 255,
        255 * 0.17742298f, 255 * 0.4375272f , 255 * 0.55756466f, 255,
        255 * 0.17584148f, 255 * 0.44128981f, 255 * 0.55768526f, 255,
        255 * 0.17427363f, 255 * 0.4450441f , 255 * 0.55779216f, 255,
        255 * 0.17271876f, 255 * 0.4487906f , 255 * 0.55788532f, 255,
        255 * 0.17127615f, 255 * 0.4525298f , 255 * 0.55796464f, 255,
        255 * 0.16964573f, 255 * 0.45626209f, 255 * 0.55803034f, 255,
        255 * 0.16812641f, 255 * 0.45998802f, 255 * 0.55808199f, 255,
        255 * 0.1666171f , 255 * 0.46370813f, 255 * 0.55812913f, 255,
        255 * 0.16512703f, 255 * 0.4674229f , 255 * 0.55814141f, 255,
        255 * 0.16362543f, 255 * 0.47123278f, 255 * 0.55814842f, 255,
        255 * 0.16214155f, 255 * 0.47483821f, 255 * 0.55813967f, 255,
        255 * 0.16066467f, 255 * 0.47853961f, 255 * 0.55812466f, 255,
        255 * 0.15919413f, 255 * 0.4822374f , 255 * 0.5580728f , 255,
        255 * 0.15772933f, 255 * 0.48593197f, 255 * 0.55801347f, 255,
        255 * 0.15626973f, 255 * 0.4896237f , 255 * 0.557936f  , 255,
        255 * 0.15481488f, 255 * 0.49331293f, 255 * 0.55783967f, 255,
        255 * 0.15336445f, 255 * 0.49700003f, 255 * 0.55772371f, 255,
        255 * 0.1519182f , 255 * 0.50068529f, 255 * 0.55758733f, 255,
        255 * 0.15047605f, 255 * 0.50436904f, 255 * 0.55742968f, 255,
        255 * 0.14903918f, 255 * 0.50805136f, 255 * 0.5572505f , 255,
        255 * 0.14760731f, 255 * 0.51273263f, 255 * 0.55704861f, 255,
        255 * 0.14618026f, 255 * 0.51541316f, 255 * 0.55682271f, 255,
        255 * 0.14475863f, 255 * 0.51909319f, 255 * 0.55657181f, 255,
        255 * 0.14334327f, 255 * 0.52277292f, 255 * 0.55629491f, 255,
        255 * 0.14193527f, 255 * 0.52645254f, 255 * 0.55599097f, 255,
        255 * 0.14053599f, 255 * 0.53013219f, 255 * 0.55565893f, 255,
        255 * 0.13914708f, 255 * 0.53381201f, 255 * 0.55529773f, 255,
        255 * 0.13777048f, 255 * 0.53749213f, 255 * 0.55490625f, 255,
        255 * 0.1364085f , 255 * 0.54127264f, 255 * 0.55448339f, 255,
        255 * 0.13506561f, 255 * 0.54485335f, 255 * 0.55402906f, 255,
        255 * 0.13374299f, 255 * 0.54853458f, 255 * 0.55354108f, 255,
        255 * 0.13244401f, 255 * 0.55221637f, 255 * 0.55301828f, 255,
        255 * 0.13127249f, 255 * 0.55589872f, 255 * 0.55245948f, 255,
        255 * 0.1299327f , 255 * 0.55958162f, 255 * 0.55186354f, 255,
        255 * 0.12872938f, 255 * 0.56326503f, 255 * 0.55122927f, 255,
        255 * 0.12756771f, 255 * 0.56694891f, 255 * 0.55055551f, 255,
        255 * 0.12645338f, 255 * 0.57063316f, 255 * 0.5498412f , 255,
        255 * 0.12539383f, 255 * 0.57431754f, 255 * 0.54908564f, 255,
        255 * 0.12439474f, 255 * 0.57800205f, 255 * 0.5482874f , 255,
        255 * 0.12346281f, 255 * 0.58168661f, 255 * 0.54744498f, 255,
        255 * 0.12260562f, 255 * 0.58537105f, 255 * 0.54655722f, 255,
        255 * 0.12183122f, 255 * 0.58905521f, 255 * 0.54562298f, 255,
        255 * 0.12124807f, 255 * 0.59273889f, 255 * 0.54464124f, 255,
        255 * 0.12056501f, 255 * 0.59642187f, 255 * 0.54361058f, 255,
        255 * 0.12009154f, 255 * 0.60010387f, 255 * 0.54253043f, 255,
        255 * 0.12973756f, 255 * 0.60378459f, 255 * 0.54139999f, 255,
        255 * 0.12951263f, 255 * 0.60746388f, 255 * 0.54021751f, 255,
        255 * 0.12942341f, 255 * 0.61214146f, 255 * 0.53898192f, 255,
        255 * 0.12948255f, 255 * 0.61481702f, 255 * 0.53769219f, 255,
        255 * 0.12969858f, 255 * 0.61849025f, 255 * 0.53634733f, 255,
        255 * 0.12008079f, 255 * 0.62216081f, 255 * 0.53494633f, 255,
        255 * 0.12063824f, 255 * 0.62582833f, 255 * 0.53348834f, 255,
        255 * 0.12137972f, 255 * 0.62949242f, 255 * 0.53197275f, 255,
        255 * 0.12231244f, 255 * 0.63315277f, 255 * 0.53039808f, 255,
        255 * 0.12344358f, 255 * 0.63680899f, 255 * 0.52876343f, 255,
        255 * 0.12477953f, 255 * 0.64046069f, 255 * 0.52706792f, 255,
        255 * 0.12632581f, 255 * 0.64410744f, 255 * 0.52531069f, 255,
        255 * 0.12808703f, 255 * 0.64774881f, 255 * 0.52349092f, 255,
        255 * 0.13006688f, 255 * 0.65138436f, 255 * 0.52160791f, 255,
        255 * 0.13226797f, 255 * 0.65501363f, 255 * 0.51966086f, 255,
        255 * 0.13469183f, 255 * 0.65863619f, 255 * 0.5176488f , 255,
        255 * 0.13733921f, 255 * 0.66225157f, 255 * 0.51557101f, 255,
        255 * 0.14020991f, 255 * 0.66585927f, 255 * 0.5134268f , 255,
        255 * 0.14330291f, 255 * 0.66945881f, 255 * 0.51221549f, 255,
        255 * 0.1466164f , 255 * 0.67304968f, 255 * 0.50893644f, 255,
        255 * 0.15014782f, 255 * 0.67663139f, 255 * 0.5065889f , 255,
        255 * 0.15389405f, 255 * 0.68020343f, 255 * 0.50417217f, 255,
        255 * 0.15785146f, 255 * 0.68376525f, 255 * 0.50168574f, 255,
        255 * 0.16201598f, 255 * 0.68731632f, 255 * 0.49912906f, 255,
        255 * 0.1663832f , 255 * 0.69085612f, 255 * 0.49650163f, 255,
        255 * 0.1709484f , 255 * 0.69438405f, 255 * 0.49380294f, 255,
        255 * 0.17570671f, 255 * 0.6978996f , 255 * 0.49103252f, 255,
        255 * 0.18065314f, 255 * 0.70140222f, 255 * 0.48818938f, 255,
        255 * 0.18578266f, 255 * 0.70489133f, 255 * 0.48527326f, 255,
        255 * 0.19109018f, 255 * 0.70836635f, 255 * 0.48228395f, 255,
        255 * 0.19657063f, 255 * 0.71282668f, 255 * 0.47922108f, 255,
        255 * 0.20221902f, 255 * 0.71527175f, 255 * 0.47608431f, 255,
        255 * 0.20803045f, 255 * 0.71870095f, 255 * 0.4728733f , 255,
        255 * 0.21400015f, 255 * 0.72212371f, 255 * 0.46958774f, 255,
        255 * 0.22012381f, 255 * 0.72550945f, 255 * 0.46622638f, 255,
        255 * 0.2263969f , 255 * 0.72888753f, 255 * 0.46278934f, 255,
        255 * 0.23281498f, 255 * 0.73224735f, 255 * 0.45927675f, 255,
        255 * 0.2393739f , 255 * 0.73558828f, 255 * 0.45568838f, 255,
        255 * 0.24606968f, 255 * 0.73890972f, 255 * 0.45202405f, 255,
        255 * 0.25289851f, 255 * 0.74221204f, 255 * 0.44828355f, 255,
        255 * 0.25985676f, 255 * 0.74549162f, 255 * 0.44446673f, 255,
        255 * 0.26694127f, 255 * 0.74875084f, 255 * 0.44057284f, 255,
        255 * 0.27414922f, 255 * 0.75198807f, 255 * 0.4366009f , 255,
        255 * 0.28147681f, 255 * 0.75520266f, 255 * 0.43255207f, 255,
        255 * 0.28892102f, 255 * 0.75839399f, 255 * 0.42842626f, 255,
        255 * 0.29647899f, 255 * 0.76156142f, 255 * 0.42422341f, 255,
        255 * 0.30414796f, 255 * 0.76470433f, 255 * 0.41994346f, 255,
        255 * 0.31292534f, 255 * 0.76782207f, 255 * 0.41558638f, 255,
        255 * 0.3198086f , 255 * 0.77091403f, 255 * 0.41215215f, 255,
        255 * 0.3277958f , 255 * 0.77397953f, 255 * 0.40664012f, 255,
        255 * 0.33588539f, 255 * 0.7770179f , 255 * 0.40204917f, 255,
        255 * 0.34407412f, 255 * 0.78002855f, 255 * 0.39738103f, 255,
        255 * 0.35235985f, 255 * 0.78301086f, 255 * 0.39263579f, 255,
        255 * 0.36074053f, 255 * 0.78596419f, 255 * 0.38781353f, 255,
        255 * 0.3692142f , 255 * 0.78888793f, 255 * 0.38291438f, 255,
        255 * 0.37777892f, 255 * 0.79178146f, 255 * 0.3779385f , 255,
        255 * 0.38643282f, 255 * 0.79464415f, 255 * 0.37288606f, 255,
        255 * 0.39517408f, 255 * 0.79747541f, 255 * 0.36775726f, 255,
        255 * 0.40400101f, 255 * 0.80027461f, 255 * 0.36255223f, 255,
        255 * 0.4129135f , 255 * 0.80304099f, 255 * 0.35726893f, 255,
        255 * 0.42190813f, 255 * 0.80577412f, 255 * 0.35191009f, 255,
        255 * 0.43098317f, 255 * 0.80847343f, 255 * 0.34647607f, 255,
        255 * 0.44013691f, 255 * 0.81213836f, 255 * 0.3409673f , 255,
        255 * 0.44936763f, 255 * 0.81376835f, 255 * 0.33538426f, 255,
        255 * 0.45867362f, 255 * 0.81636288f, 255 * 0.32972749f, 255,
        255 * 0.46805314f, 255 * 0.81892143f, 255 * 0.32399761f, 255,
        255 * 0.47750446f, 255 * 0.82144351f, 255 * 0.31819529f, 255,
        255 * 0.4870258f , 255 * 0.82392862f, 255 * 0.31232133f, 255,
        255 * 0.49661536f, 255 * 0.82637633f, 255 * 0.30637661f, 255,
        255 * 0.5062713f , 255 * 0.82878621f, 255 * 0.30036212f, 255,
        255 * 0.51599182f, 255 * 0.83125784f, 255 * 0.29427888f, 255,
        255 * 0.52577622f, 255 * 0.83349064f, 255 * 0.2881265f , 255,
        255 * 0.5356212f , 255 * 0.83578452f, 255 * 0.28190832f, 255,
        255 * 0.5455244f , 255 * 0.83803918f, 255 * 0.27562602f, 255,
        255 * 0.55548397f, 255 * 0.84025437f, 255 * 0.26928147f, 255,
        255 * 0.5654976f , 255 * 0.8424299f , 255 * 0.26287683f, 255,
        255 * 0.57556297f, 255 * 0.84456561f, 255 * 0.25641457f, 255,
        255 * 0.58567772f, 255 * 0.84666139f, 255 * 0.24989748f, 255,
        255 * 0.59583934f, 255 * 0.84871722f, 255 * 0.24332878f, 255,
        255 * 0.60604528f, 255 * 0.8507331f , 255 * 0.23671214f, 255,
        255 * 0.61629283f, 255 * 0.85270912f, 255 * 0.23005179f, 255,
        255 * 0.62657923f, 255 * 0.85464543f, 255 * 0.22335258f, 255,
        255 * 0.63690157f, 255 * 0.85654226f, 255 * 0.21662012f, 255,
        255 * 0.64725685f, 255 * 0.85839991f, 255 * 0.20986086f, 255,
        255 * 0.65764197f, 255 * 0.86021878f, 255 * 0.20308229f, 255,
        255 * 0.66805369f, 255 * 0.86199932f, 255 * 0.19629307f, 255,
        255 * 0.67848868f, 255 * 0.86374212f, 255 * 0.18950326f, 255,
        255 * 0.68894351f, 255 * 0.86544779f, 255 * 0.18272455f, 255,
        255 * 0.69941463f, 255 * 0.86712712f, 255 * 0.17597055f, 255,
        255 * 0.70989842f, 255 * 0.86875092f, 255 * 0.16925712f, 255,
        255 * 0.72039125f, 255 * 0.87035015f, 255 * 0.16260273f, 255,
        255 * 0.73088902f, 255 * 0.87191584f, 255 * 0.15602894f, 255,
        255 * 0.74138803f, 255 * 0.87344918f, 255 * 0.14956101f, 255,
        255 * 0.75188414f, 255 * 0.87495143f, 255 * 0.14322828f, 255,
        255 * 0.76237342f, 255 * 0.87642392f, 255 * 0.13706449f, 255,
        255 * 0.77285183f, 255 * 0.87786808f, 255 * 0.13120864f, 255,
        255 * 0.78331535f, 255 * 0.87928545f, 255 * 0.12540538f, 255,
        255 * 0.79375994f, 255 * 0.88067763f, 255 * 0.12000532f, 255,
        255 * 0.80418159f, 255 * 0.88204632f, 255 * 0.12496505f, 255,
        255 * 0.81457634f, 255 * 0.88339329f, 255 * 0.12034678f, 255,
        255 * 0.82494028f, 255 * 0.88472036f, 255 * 0.10621724f, 255,
        255 * 0.83526959f, 255 * 0.88602943f, 255 * 0.1026459f , 255,
        255 * 0.84556056f, 255 * 0.88732243f, 255 * 0.09970219f, 255,
        255 * 0.8558096f , 255 * 0.88860134f, 255 * 0.09745186f, 255,
        255 * 0.86601325f, 255 * 0.88986815f, 255 * 0.09595277f, 255,
        255 * 0.87616824f, 255 * 0.89122487f, 255 * 0.09525046f, 255,
        255 * 0.88627146f, 255 * 0.89237353f, 255 * 0.09537439f, 255,
        255 * 0.89632002f, 255 * 0.89361614f, 255 * 0.09633538f, 255,
        255 * 0.90631221f, 255 * 0.89485467f, 255 * 0.09812496f, 255,
        255 * 0.91624212f, 255 * 0.89609127f, 255 * 0.1007168f , 255,
        255 * 0.92610579f, 255 * 0.89732977f, 255 * 0.10407067f, 255,
        255 * 0.93590444f, 255 * 0.8985704f , 255 * 0.10813094f, 255,
        255 * 0.94563626f, 255 * 0.899815f  , 255 * 0.12283773f, 255,
        255 * 0.95529972f, 255 * 0.90106534f, 255 * 0.12812832f, 255,
        255 * 0.96489353f, 255 * 0.90232312f, 255 * 0.12394051f, 255,
        255 * 0.97441665f, 255 * 0.90358991f, 255 * 0.13021494f, 255,
        255 * 0.98386829f, 255 * 0.90486726f, 255 * 0.13689671f, 255,
        255 * 0.99324789f, 255 * 0.90615657f, 255 * 0.1439362f , 255
    };
#pragma warning(default: 4244)
#pragma warning(default: 4838)

    const auto size = std::size(data);
    auto retval = create_upload_buffer(device, size);
    set_debug_object_name(retval, "viridis_colour_map_upload");
    stage_data(retval.get(), data, size);
    return retval;
}


/*
 * trrojan::d3d12::create_viridis_colour_map
 */
winrt::com_ptr<ID3D12Resource> trrojan::d3d12::create_viridis_colour_map(
        device& device, ID3D12GraphicsCommandList *cmd_list,
        const D3D12_RESOURCE_STATES state) {
    assert(cmd_list != nullptr);
    auto upload = create_viridis_colour_map(device.d3d_device());
    const auto size = upload->GetDesc().Width / (4 * sizeof(BYTE));
    auto retval = create_texture(device.d3d_device(), size,
        DXGI_FORMAT_R8G8B8A8_UNORM);
    set_debug_object_name(retval, "viridis_colour_map");

    auto src_loc = get_copy_location(upload.get());
    // "reinterpret_cast" the source ...
    assert(src_loc.PlacedFootprint.Footprint.Format == DXGI_FORMAT_UNKNOWN);
    src_loc.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    src_loc.PlacedFootprint.Footprint.Width /= 4;
    auto dst_loc = get_copy_location(retval.get());

    cmd_list->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);
    transition_resource(cmd_list, retval.get(), D3D12_RESOURCE_STATE_COPY_DEST,
        state);
    device.close_and_execute_command_list(cmd_list);
    device.wait_for_gpu();

    return retval;
}


/*
 * trrojan::d3d12::get_adapter
 */
winrt::com_ptr<IDXGIAdapter> trrojan::d3d12::get_adapter(ID3D12Device *device,
        IDXGIFactory4 *factory) {
    check_not_null(device);
    winrt::com_ptr<IDXGIFactory4> f;
    winrt::com_ptr<IDXGIAdapter> retval;

    if (f == nullptr) {
        auto hr = ::CreateDXGIFactory2(0, IID_IDXGIFactory4,
            reinterpret_cast<void **>(&f));
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    } else {
        f.copy_from(factory);
    }

    auto hr = f->EnumAdapterByLuid(device->GetAdapterLuid(),
        IID_IDXGIAdapter, reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::get_device
 */
winrt::com_ptr<ID3D12Device> trrojan::d3d12::get_device(
        ID3D12DeviceChild *child) {
    check_not_null(child);
    winrt::com_ptr<ID3D12Device> retval;

    auto hr = child->GetDevice(::IID_ID3D12Device,
        reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::get_copy_location
 */
D3D12_TEXTURE_COPY_LOCATION trrojan::d3d12::get_copy_location(
        ID3D12Resource *resource, const UINT subresource) {
    auto device = get_device(resource);
    auto desc = resource->GetDesc();

    D3D12_TEXTURE_COPY_LOCATION retval;
    ::ZeroMemory(&retval, sizeof(retval));

    retval.pResource = resource;

    if (desc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        retval.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        device->GetCopyableFootprints(&desc, 0, 1, 0, &retval.PlacedFootprint,
            nullptr, nullptr, nullptr);

    } else {
        retval.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        retval.SubresourceIndex = subresource;
    }

    return retval;
}


/*
 * trrojan::d3d12::get_file_path
 */
std::wstring trrojan::d3d12::get_file_path(HANDLE handle) {
    std::vector<wchar_t> retval(MAX_PATH + 1);

    auto cnt = ::GetFinalPathNameByHandleW(handle, retval.data(),
        static_cast<DWORD>(retval.size()), FILE_NAME_NORMALIZED);
    if (cnt > retval.size()) {
        retval.resize(cnt);
        cnt = ::GetFinalPathNameByHandleW(handle, retval.data(),
            static_cast<DWORD>(retval.size()), FILE_NAME_NORMALIZED);
    }

    if (cnt == 0) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return std::wstring(retval.data(), retval.data() + cnt);
}


#if !defined(TRROJAN_FOR_UWP)
/*
 * trrojan::d3d12::get_mapped_file_path
 */
std::wstring trrojan::d3d12::get_mapped_file_path(void *address) {
    std::vector<wchar_t> retval(MAX_PATH + 1);

    auto cnt = ::GetMappedFileNameW(::GetCurrentProcess(), address,
        retval.data(), static_cast<DWORD>(retval.size()));
    if (cnt == 0) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return std::wstring(retval.data(), retval.data() + cnt);
}
#endif /* !defined(TRROJAN_FOR_UWP) */


/*
 * trrojan::d3d12::get_video_memory_info
 */
DXGI_QUERY_VIDEO_MEMORY_INFO trrojan::d3d12::get_video_memory_info(
        ID3D12Device *device, IDXGIFactory4 *factory, const UINT node_index,
        const DXGI_MEMORY_SEGMENT_GROUP segment_group) {
    winrt::com_ptr<IDXGIAdapter3> adapter3;
    DXGI_QUERY_VIDEO_MEMORY_INFO retval;

    {
        auto adapter = get_adapter(device, factory);
        if (!adapter.try_as(adapter3)) {
            throw std::system_error(E_NOINTERFACE, com_category());
        }
    }

    {
        auto hr = adapter3->QueryVideoMemoryInfo(node_index, segment_group,
            &retval);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }

    return retval;
}


/*
 * trrojan::d3d12::map_view_of_file
 */
std::unique_ptr<void, trrojan::memory_unmapper>
trrojan::d3d12::map_view_of_file(winrt::handle& mapping, const DWORD access,
        const std::size_t offset, const std::size_t size) {
    assert(mapping);
    ULARGE_INTEGER o;
    o.QuadPart = offset;

    std::unique_ptr<void, trrojan::memory_unmapper> retval(::MapViewOfFile(
        mapping.get(), access, o.HighPart, o.LowPart, size));
    if (retval == nullptr) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return retval;
}


/*
 * trrojan::d3d12::map_view_of_file
 */
std::pair<std::unique_ptr<void, trrojan::memory_unmapper>, void *>
trrojan::d3d12::map_view_of_file(winrt::handle& mapping,
        const std::size_t allocation_granularity,
        const DWORD access,
        const std::size_t offset,
        const std::size_t size) {
    const auto pad = (offset % allocation_granularity);
    assert(pad <= offset);
    auto view = d3d12::map_view_of_file(mapping, access, offset - pad,
        size + pad);
    auto pos = static_cast<std::uint8_t *>(view.get()) + pad;
    return std::make_pair(std::move(view), pos);
}


/*
 * trrojan::d3d12::set_debug_object_name
 */
void trrojan::d3d12::set_debug_object_name(ID3D12Object *obj,
        const char *name) {
    if ((obj != nullptr) && (name != nullptr)) {
        auto len = static_cast<UINT>(::strlen(name));

        auto hr = obj->SetPrivateData(WKPDID_D3DDebugObjectName, len, name);
        if (FAILED(hr)) {
            throw std::system_error(hr, com_category());
        }
    }
}


/*
 * trrojan::d3d12::stage_data
 */
void trrojan::d3d12::stage_data(ID3D12Resource *resource, const void *data,
        const UINT64 cnt) {
    check_not_null(data);
    stage_data(resource, [data, cnt](void *dst, const UINT64 size) {
        if (size < cnt) {
            throw std::invalid_argument("The staging buffer is too small for "
                "the amount of data specified.");
        }

        ::memcpy(dst, data, cnt);
    });
}


/*
 * trrojan::d3d12::stage_data
 */
void trrojan::d3d12::stage_data(ID3D12Resource *resource,
        const std::function<void(void *, const UINT64)>& producer) {
    check_staging_buffer(resource);
    assert(resource != nullptr);
    if (!producer) {
        throw std::invalid_argument("A valid producer callback must be "
            "specified.");
    }

    BYTE *dst;
    D3D12_RANGE nothing = { 0, 0 };
    auto hr = resource->Map(0, &nothing, reinterpret_cast<void **>(&dst));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    auto device = trrojan::d3d12::get_device(resource);
    UINT64 size;
    device->GetCopyableFootprints(&resource->GetDesc(), 0, 1, 0, nullptr,
        nullptr, nullptr, &size);

    producer(dst, size);

    resource->Unmap(0, nullptr);
}


#if 0
inline void MemcpySubresource(
    _In_ const D3D12_MEMCPY_DEST *pDest,
    _In_ const D3D12_SUBRESOURCE_DATA *pSrc,
    SIZE_T RowSizeInBytes,
    UINT NumRows,
    UINT NumSlices)
{
    for (UINT z = 0; z < NumSlices; ++z)
    {
        BYTE *pDestSlice = reinterpret_cast<BYTE *>(pDest->pData) + pDest->SlicePitch * z;
        const BYTE *pSrcSlice = reinterpret_cast<const BYTE *>(pSrc->pData) + pSrc->SlicePitch * z;
        for (UINT y = 0; y < NumRows; ++y)
        {
            memcpy(pDestSlice + pDest->RowPitch * y,
                pSrcSlice + pSrc->RowPitch * y,
                RowSizeInBytes);
        }
    }
}
#endif

/*
 * trrojan::d3d12::stage_data
 */
void trrojan::d3d12::stage_data(ID3D12Resource *resource, const void *data,
        const UINT64 cnt_cols, const UINT cnt_rows, UINT64 row_pitch) {
    check_staging_buffer(resource, cnt_cols * cnt_rows);

    if (row_pitch < cnt_cols) {
        row_pitch = cnt_cols;
    }

    BYTE *dst;
    auto hr = resource->Map(0, nullptr, reinterpret_cast<void **>(&dst));
    if (FAILED(hr)) {
        throw std::system_error(hr, com_category());
    }

    auto src = static_cast<const BYTE *>(data);

    for (UINT64 r = 0; r < cnt_rows; ++r) {
        ::memcpy(dst + r * cnt_cols, src + r * row_pitch, cnt_cols);
    }

    resource->Unmap(0, nullptr);
}


/*
 * trrojan::d3d12::transition_resource
 */
void trrojan::d3d12::transition_resource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *resource, const D3D12_RESOURCE_STATES state_before,
        const D3D12_RESOURCE_STATES state_after) {
    transition_subresource(cmd_list, resource,
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state_before, state_after);
}


/*
 * trrojan::d3d12::transition_subresource
 */
void trrojan::d3d12::transition_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *resource, const UINT subresource,
        const D3D12_RESOURCE_STATES state_before,
        const D3D12_RESOURCE_STATES state_after) {
    assert(cmd_list != nullptr);
#if (defined(DEBUG) || defined(_DEBUG))
    log::instance().write_line(log_level::debug, "transition_subresource("
        "{0:p}, {1}, {2}, {3})", static_cast<void *>(cmd_list), subresource,
        static_cast<unsigned int>(state_before),
        static_cast<unsigned int>(state_after));
#endif  /* (defined(DEBUG) || defined(_DEBUG)) */

    D3D12_RESOURCE_BARRIER barrier;
    ::ZeroMemory(&barrier, sizeof(barrier));
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource;
    barrier.Transition.StateBefore = state_before;
    barrier.Transition.StateAfter = state_after;
    barrier.Transition.Subresource = subresource;

    cmd_list->ResourceBarrier(1, &barrier);
}


/*
 * trrojan::d3d12::update_subresource
 */
void trrojan::d3d12::update_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *dst, const UINT subresource,
        const D3D12_RESOURCE_STATES state_after, ID3D12Resource *staging,
        const void *data, const UINT64 cnt) {
    check_not_null(cmd_list);
    check_not_null(dst);

    stage_data(staging, data, cnt);

    if (dst->GetDesc().Dimension == D3D12_RESOURCE_DIMENSION_BUFFER) {
        cmd_list->CopyBufferRegion(dst, 0, staging, 0, cnt);

    } else {
        auto dst_loc = get_copy_location(dst, subresource);
        auto src_loc = get_copy_location(staging, 0);
        cmd_list->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);
    }

    if (state_after != D3D12_RESOURCE_STATE_COPY_DEST) {
        transition_subresource(cmd_list, dst, subresource,
            D3D12_RESOURCE_STATE_COPY_DEST, state_after);
    }
}


/*
 * trrojan::d3d12::update_subresource
 */
void trrojan::d3d12::update_subresource(ID3D12GraphicsCommandList *cmd_list,
        ID3D12Resource *dst, const UINT subresource,
        const D3D12_RESOURCE_STATES state_after, ID3D12Resource *staging,
        const void *data, const UINT64 cnt_cols, const UINT cnt_rows,
        const UINT64 row_pitch) {
    check_not_null(cmd_list);
    check_not_null(dst);

    stage_data(staging, data, cnt_cols, cnt_rows, row_pitch);

    auto dst_loc = get_copy_location(dst, subresource);
    auto src_loc = get_copy_location(staging, 0);
    cmd_list->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);

    if (state_after != D3D12_RESOURCE_STATE_COPY_DEST) {
        transition_subresource(cmd_list, dst, subresource,
            D3D12_RESOURCE_STATE_COPY_DEST, state_after);
    }
}


/*
 * trrojan::d3d12::wait_for_event
 */
void trrojan::d3d12::wait_for_event(handle<>& handle) {
    switch (::WaitForSingleObjectEx(handle, INFINITE, FALSE)) {
        case WAIT_FAILED:
            throw std::system_error(::GetLastError(), std::system_category());

        default:
            break;
    }
}
