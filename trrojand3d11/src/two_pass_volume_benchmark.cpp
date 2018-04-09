/// <copyright file="two_pass_volume_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/two_pass_volume_benchmark.h"


/*
 * trrojan::d3d11::two_pass_volume_benchmark::two_pass_volume_benchmark
 */
trrojan::d3d11::two_pass_volume_benchmark::two_pass_volume_benchmark(void)
    : volume_benchmark_base("2pass-volume-renderer") { }


/*
 * trrojan::d3d11::two_pass_volume_benchmark::on_run
 */
trrojan::result trrojan::d3d11::two_pass_volume_benchmark::on_run(
        d3d11::device& device, const configuration& config,
        const std::vector<std::string>& changed) {
    volume_benchmark_base::on_run(device, config, changed);

    const auto dev = device.d3d_device();
    const auto vp = config.get<viewport_type>(factor_viewport);

    if (contains(changed, factor_device)) {
        this->rasteriser_state = nullptr;

        {
            D3D11_RASTERIZER_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.FillMode = D3D11_FILL_SOLID;
            desc.CullMode = D3D11_CULL_NONE;

            auto hr = dev->CreateRasterizerState(&desc,
                &this->rasteriser_state);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }

    // If the device or the viewport changed, invalidate the intermediate
    // buffers.
    if (contains_any(changed, factor_device, factor_viewport)) {
        this->back_faces = nullptr;
        this->front_faces = nullptr;
    }

    if (this->back_faces == nullptr) {
        assert(this->front_faces == nullptr);
        D3D11_TEXTURE2D_DESC desc;

        ::ZeroMemory(&desc, sizeof(desc));
        desc.ArraySize = 1;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        desc.Height = vp[1];
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.Width = vp[0];

        {
            auto hr = dev->CreateTexture2D(&desc, nullptr, &this->back_faces);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            auto hr = dev->CreateTexture2D(&desc, nullptr, &this->front_faces);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }

    // TODO: implement the benchmark.

    return nullptr;
}
