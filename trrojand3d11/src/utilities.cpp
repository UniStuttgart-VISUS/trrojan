/// <copyright file="utilities.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/utilities.h"


/*
 * trrojan::d3d11::create_buffer
 */
ATL::CComPtr<ID3D11Buffer> trrojan::d3d11::create_buffer(ID3D11Device *device,
        const D3D11_USAGE usage, const D3D11_BIND_FLAG binding,
        const void *data, const UINT cntData, const UINT cpuAccess) {
    assert(device != nullptr);
    D3D11_BUFFER_DESC bufferDesc;
    D3D11_SUBRESOURCE_DATA id;
    ATL::CComPtr<ID3D11Buffer> retval;

    ::ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = static_cast<UINT>(cntData);
    bufferDesc.Usage = usage;
    bufferDesc.BindFlags = binding;
    bufferDesc.CPUAccessFlags = cpuAccess;

    if (data != nullptr) {
        ::ZeroMemory(&id, sizeof(id));
        id.pSysMem = data;
    }

    auto hr = device->CreateBuffer(&bufferDesc,
        (data != nullptr) ? &id : nullptr, &retval);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to create buffer with error " << hr << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
