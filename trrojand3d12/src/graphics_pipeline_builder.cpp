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
trrojan::d3d12::graphics_pipeline_builder::graphics_pipeline_builder(void) {
    ::ZeroMemory(&this->_desc, sizeof(this->_desc));
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::build
 */
ATL::CComPtr<ID3D12PipelineState>
trrojan::d3d12::graphics_pipeline_builder::build(ID3D12Device *device) {
    if (device == nullptr) {
        throw ATL::CAtlException(E_POINTER);
    }

    ATL::CComPtr<ID3D12PipelineState> retval;
    auto hr = device->CreateGraphicsPipelineState(&this->_desc,
        IID_PPV_ARGS(&retval));
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::reset_depth_stencil_state
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::reset_depth_stencil_state(void) {
    this->_desc.DepthStencilState.DepthEnable = TRUE;
    this->_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    this->_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    this->_desc.DepthStencilState.StencilEnable = FALSE;
    this->_desc.DepthStencilState.StencilReadMask
        = D3D12_DEFAULT_STENCIL_READ_MASK;
    this->_desc.DepthStencilState.StencilWriteMask
        = D3D12_DEFAULT_STENCIL_WRITE_MASK;
    this->_desc.DepthStencilState.FrontFace.StencilFailOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.BackFace.StencilFailOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.FrontFace.StencilDepthFailOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.BackFace.StencilDepthFailOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.FrontFace.StencilPassOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.BackFace.StencilPassOp
        = D3D12_STENCIL_OP_KEEP;
    this->_desc.DepthStencilState.FrontFace.StencilFunc
        = D3D12_COMPARISON_FUNC_ALWAYS;
    this->_desc.DepthStencilState.BackFace.StencilFunc
        = D3D12_COMPARISON_FUNC_ALWAYS;
    return *this;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::reset_rasteriser_state
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::reset_rasteriser_state(void) {
    this->_desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    this->_desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    this->_desc.RasterizerState.FrontCounterClockwise = FALSE;
    this->_desc.RasterizerState.DepthBias = 0;
    this->_desc.RasterizerState.DepthBiasClamp = 0.0f;
    this->_desc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    this->_desc.RasterizerState.DepthClipEnable = TRUE;
    this->_desc.RasterizerState.MultisampleEnable = FALSE;
    this->_desc.RasterizerState.AntialiasedLineEnable = FALSE;
    this->_desc.RasterizerState.ForcedSampleCount = 0;
    this->_desc.RasterizerState.ConservativeRaster
        = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
    return *this;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::reset_shaders
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::reset_shaders(void) {
    this->_desc.DS.BytecodeLength
        = this->_desc.GS.BytecodeLength
        = this->_desc.HS.BytecodeLength
        = this->_desc.PS.BytecodeLength
        = this->_desc.VS.BytecodeLength
        = 0;
    this->_desc.DS.pShaderBytecode
        = this->_desc.GS.pShaderBytecode
        = this->_desc.HS.pShaderBytecode
        = this->_desc.PS.pShaderBytecode
        = this->_desc.VS.pShaderBytecode
        = nullptr;
    return *this;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::set_render_targets
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::set_render_targets(
        const std::vector<DXGI_FORMAT>& formats) {
    this->_desc.NumRenderTargets = static_cast<UINT>(std::min(
        std::size(this->_desc.RTVFormats), formats.size()));
    std::copy_n(formats.begin(), this->_desc.NumRenderTargets,
        this->_desc.RTVFormats);
    return *this;
}


/*
 * trrojan::d3d12::graphics_pipeline_builder::set_root_signature
 */
trrojan::d3d12::graphics_pipeline_builder&
trrojan::d3d12::graphics_pipeline_builder::set_root_signature(
        ID3D12RootSignature *root_signature) {
    // Implementation note: is is much easier for us to keep a smart pointer
    // reference in the form of '_root_sig' instead of manually managing the
    // reference count of the pointer in '_desc', which would require custom
    // copy ctors and assignment operators.
    this->_root_sig = root_signature;
    this->_desc.pRootSignature = this->_root_sig;
    return *this;
}
