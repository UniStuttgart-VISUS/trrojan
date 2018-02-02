/// <copyright file="rendering_technique.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/rendering_technique.h"

#include <algorithm>
#include <cassert>


/*
 * trrojan::d3d11::rendering_technique::apply
 */
void trrojan::d3d11::rendering_technique::apply(ID3D11DeviceContext *ctx) {
    assert(ctx != nullptr);

    this->set_constant_buffers(1, 0, ATL::CComPtr<ID3D11Buffer>());

    /* Configure input. */
    ctx->IASetInputLayout(this->inputLayout);
    {
        auto vbs = unsmart(this->vertexBuffers);
        ctx->IASetVertexBuffers(0, static_cast<UINT>(vbs.size()), vbs.data(),
            NULL, NULL); // TODO
        ctx->IASetPrimitiveTopology(this->primitiveTopology);
    }

    /* Configure shaders. */
    ctx->VSSetShader(this->vertexShader.p, nullptr, 0);
    ctx->DSSetShader(this->domainShader.p, nullptr, 0);
    ctx->HSSetShader(this->hullShader, nullptr, 0);
    ctx->GSSetShader(this->geometryShader, nullptr, 0);
    ctx->PSSetShader(this->pixelShader, nullptr, 0);

    /* Bind shader resources. */
    for (auto& res : this->_resources) {
        std::vector<ID3D11Buffer *> cbs;
        std::vector<ID3D11ShaderResourceView *> srvs;

        switch (res.first) {
            case shader_stage::vertex:
                cbs = unsmart(res.second.constant_buffers);
                srvs = unsmart(res.second.resource_views);
                ctx->VSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                    cbs.data());
                ctx->VSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                    srvs.data());
                break;

            case shader_stage::hull:
                cbs = unsmart(res.second.constant_buffers);
                ctx->HSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                    cbs.data());
                srvs = unsmart(res.second.resource_views);
                ctx->HSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                    srvs.data());
                break;

            case shader_stage::domain:
                cbs = unsmart(res.second.constant_buffers);
                ctx->DSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                    cbs.data());
                srvs = unsmart(res.second.resource_views);
                ctx->DSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                    srvs.data());
                break;

            case shader_stage::geometry:
                cbs = unsmart(res.second.constant_buffers);
                ctx->GSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                    cbs.data());
                srvs = unsmart(res.second.resource_views);
                ctx->GSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                    srvs.data());
                break;

            case shader_stage::pixel:
                cbs = unsmart(res.second.constant_buffers);
                ctx->PSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                    cbs.data());
                srvs = unsmart(res.second.resource_views);
                ctx->PSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                    srvs.data());
                break;
        }
    }
}
