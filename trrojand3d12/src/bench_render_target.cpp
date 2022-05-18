// <copyright file="bench_render_target.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2012 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/bench_render_target.h"

#include <cassert>
#include <sstream>

#include "trrojan/d3d12/utilities.h"


/*
 * trrojan::d3d12::bench_render_target::bench_render_target
 */
trrojan::d3d12::bench_render_target::bench_render_target(
        const trrojan::device& device) : base(device) {
    assert(this->device() != nullptr);
}


/*
 * trrojan::d3d12::bench_render_target::resize
 */
void trrojan::d3d12::bench_render_target::resize(
        const unsigned int width, const unsigned int height) {
#if TODO
    ATL::CComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = S_OK;
    D3D11_TEXTURE2D_DESC texDesc;

    // TODO: could optimise this to prevent unnecessary re-creates.
    ::ZeroMemory(&texDesc, sizeof(texDesc));
    texDesc.ArraySize = 1;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
    // UAV does not support BGRA: texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.Width = width;

    hr = this->device()->CreateTexture2D(&texDesc, nullptr, &backBuffer);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    set_debug_object_name(backBuffer.p, "bench_render_target (colour buffer)");

    // Update the back buffer and all of its views.
    this->_dsv = nullptr;
    this->_rtv = nullptr;
    this->set_back_buffer(backBuffer.p);
#endif
}
