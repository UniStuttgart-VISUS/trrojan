// <copyright file="graphics_pipeline_builder.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/graphics_pipeline_builder.h"

#include "trrojan/log.h"


/*
 * trrojan::d3d12::graphics_pipeline_builder::root_signature_from_shader
 */
ATL::CComPtr<ID3D12RootSignature>
trrojan::d3d12::graphics_pipeline_builder::root_signature_from_shader(
        ID3D12Device *device, const std::vector<BYTE>& byte_code) {
    if (device == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    ATL::CComPtr<ID3D12RootSignature> retval;
    auto hr = device->CreateRootSignature(0, byte_code.data(), byte_code.size(),
        ::IID_ID3D12RootSignature, reinterpret_cast<void **>(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::root_signature_from_shader
 */
ATL::CComPtr<ID3D12RootSignature>
trrojan::d3d12::graphics_pipeline_builder::root_signature_from_shader(
        ID3D12Device *device, const graphics_pipeline_builder& builder) {
    try {
        return root_signature_from_shader(device, builder._vs);
    } catch (...) {
        log::instance().write_line(log_level::warning, "Vertex shader has no "
            "root signature embedded, trying next shader stage.");
    }

    try {
        return root_signature_from_shader(device, builder._hs);
    } catch (...) { 
        log::instance().write_line(log_level::warning, "Hull shader has no "
            "root signature embedded, trying next shader stage.");
    }

    try {
        return root_signature_from_shader(device, builder._ds);
    } catch (...) {
        log::instance().write_line(log_level::warning, "Domain shader has no "
            "root signature embedded, trying next shader stage.");
    }

    try {
        return root_signature_from_shader(device, builder._gs);
    } catch (...) {
        log::instance().write_line(log_level::warning, "Geometry shader has no "
            "root signature embedded, trying next shader stage.");
    }

    try {
        return root_signature_from_shader(device, builder._ps);
    } catch (...) {
        log::instance().write_line(log_level::warning, "Pixel shader has no "
            "root signature embedded, no root signature is available in the "
            "current pipeline.");
    }

    return nullptr;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::graphics_pipeline_builder
 */
trrojan::d3d12::graphics_pipeline_builder::graphics_pipeline_builder(void)
    : _stream(aligned_allocator<BYTE>(sizeof(void *))) { }


/*
 * trrojan::d3d12::graphics_pipeline_builder::build
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::graphics_pipeline_builder::build(ID3D12Device2 *device) {
    if (device == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    D3D12_PIPELINE_STATE_STREAM_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.SizeInBytes = this->_stream.size();
    desc.pPipelineStateSubobjectStream = this->_stream.data();

#if (defined(DEBUG) || defined(_DEBUG))
    {
        CD3DX12_PIPELINE_STATE_STREAM_PARSE_HELPER helper;
        auto hr = D3DX12ParsePipelineStream(desc, &helper);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

    ATL::CComPtr<ID3D12PipelineState> retval;
    auto hr = device->CreatePipelineState(&desc, IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::build
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::graphics_pipeline_builder::build(ID3D12Device *device) {
    if (device == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    ATL::CComPtr<ID3D12Device2> dev;
    auto hr = device->QueryInterface(::IID_ID3D12Device2,
        reinterpret_cast<void **>(&dev));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return this->build(dev);
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::reset_shaders
 */
void trrojan::d3d12::graphics_pipeline_builder::reset_shaders(void) {
    static constexpr D3D12_PIPELINE_STATE_SUBOBJECT_TYPE shaders[] = {
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS,
        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS
    };

    for (auto& s : shaders) {
        std::size_t offset = 0;
        std::size_t size = 0;

        this->foreach_subobject([s, &offset, &size](
                const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE t,
                const std::size_t o,
                const std::size_t s) {
            if (s == t) {
                offset = o;
                size = s;
                return false;
            }

            return true;
        });

        if (size > 0) {
            this->_stream.erase(this->_stream.begin() + offset,
                this->_stream.begin() + offset + size);
        }
    }
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::set_render_targets
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::set_render_targets(
        const std::vector<DXGI_FORMAT>& formats) {
    auto& so = this->get_value<
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS>();
    so.NumRenderTargets = static_cast<UINT>(std::min(
        std::size(so.RTFormats), formats.size()));
    std::copy_n(formats.begin(), so.NumRenderTargets, so.RTFormats);
    return *this;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::set_root_signature
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::set_root_signature(
        ID3D12RootSignature *root_signature) {
    auto& so = this->get_value<
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE>();
    // Implementation note: is is much easier for us to keep a smart pointer
    // reference in the form of '_root_sig' instead of manually managing the
    // reference count of the pointer in '_desc', which would require custom
    // copy ctors and assignment operators.
    this->_root_sig = root_signature;
    so = this->_root_sig;
    return *this;
}
