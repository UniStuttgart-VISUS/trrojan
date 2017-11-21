/// <copyright file="bench_render_target.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/bench_render_target.h"

#include <cassert>
#include <sstream>


/*
 * trrojan::d3d11::bench_render_target::bench_render_target
 */
trrojan::d3d11::bench_render_target::bench_render_target(
        const trrojan::device& device) : base(device) {
    assert(this->device() != nullptr);
    assert(this->device_context() != nullptr);
}


/*
 * trrojan::d3d11::bench_render_target::resize
 */
void trrojan::d3d11::bench_render_target::resize(
        const unsigned int width, const unsigned int height) {
    ATL::CComPtr<ID3D11Texture2D> backBuffer;
    D3D11_TEXTURE2D_DESC texDesc;
    D3D11_RENDER_TARGET_VIEW_DESC viewDesc;

    // TODO: could optimise this to prevent unnecessary re-creates.
    {
        ::ZeroMemory(&texDesc, sizeof(texDesc));
        texDesc.ArraySize = 1;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
        texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.Width = width;

        auto hr = this->device()->CreateTexture2D(&texDesc, nullptr,
            &backBuffer);
        if (FAILED(hr)) {
            std::stringstream msg;
            msg << "Failed to allocate frame buffer with error code " << hr
                << "." << std::ends;
            throw std::runtime_error(msg.str());
        }
    }

    // Update the back buffer and all of its views.
    this->_dsv = nullptr;
    this->_rtv = nullptr;
    this->set_back_buffer(backBuffer.p);

}
