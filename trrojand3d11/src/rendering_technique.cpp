/// <copyright file="rendering_technique.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/rendering_technique.h"

#include <algorithm>
#include <cassert>


/*
 * trrojan::d3d11::rendering_technique::set_shader_resource_view
 */
void trrojan::d3d11::rendering_technique::set_shader_resource_view(
        shader_resources& res, srv_type srv, const UINT idx) {
    assert_range(res.resource_views, idx);
    res.resource_views[idx] = srv;
}


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(void)
    : primitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED) { }


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, std::vector<vertex_buffer>&& vbs,
        ID3D11InputLayout *il, const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11HullShader* hs, shader_resources&& hsRes,
        ID3D11DomainShader *ds, shader_resources&& dsRes,
        ID3D11GeometryShader *gs, shader_resources&& gsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : domainShader(ds), geometryShader(gs), hullShader(hs), inputLayout(il),
        _name(name), pixelShader(ps), primitiveTopology(pt),
        vertexBuffers(std::move(vbs)), vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::hull] = std::move(hsRes);
    this->_resources[shader_stage::domain] = std::move(dsRes);
    this->_resources[shader_stage::geometry] = std::move(gsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, const vertex_buffer& vb,
        ID3D11InputLayout *il, const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11HullShader* hs, shader_resources&& hsRes,
        ID3D11DomainShader *ds, shader_resources&& dsRes,
        ID3D11GeometryShader *gs, shader_resources&& gsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : domainShader(ds), geometryShader(gs), hullShader(hs), inputLayout(il),
        _name(name), pixelShader(ps), primitiveTopology(pt),
        vertexBuffers(1, vb), vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::hull] = std::move(hsRes);
    this->_resources[shader_stage::domain] = std::move(dsRes);
    this->_resources[shader_stage::geometry] = std::move(gsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, ID3D11InputLayout *il,
        const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11HullShader* hs, shader_resources&& hsRes,
        ID3D11DomainShader *ds, shader_resources&& dsRes,
        ID3D11GeometryShader *gs, shader_resources&& gsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : domainShader(ds), geometryShader(gs), hullShader(hs), inputLayout(il),
        _name(name), pixelShader(ps), primitiveTopology(pt), vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::hull] = std::move(hsRes);
    this->_resources[shader_stage::domain] = std::move(dsRes);
    this->_resources[shader_stage::geometry] = std::move(gsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}

/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, ID3D11InputLayout *il,
        const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : inputLayout(il), _name(name), pixelShader(ps), primitiveTopology(pt),
        vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, ID3D11InputLayout *il,
        const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11GeometryShader *gs, shader_resources&& gsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : geometryShader(gs), inputLayout(il),  _name(name), pixelShader(ps),
        primitiveTopology(pt), vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::geometry] = std::move(gsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}


/*
 * trrojan::d3d11::rendering_technique::rendering_technique
 */
trrojan::d3d11::rendering_technique::rendering_technique(
        const std::string& name, ID3D11InputLayout *il,
        const D3D11_PRIMITIVE_TOPOLOGY pt,
        ID3D11VertexShader *vs, shader_resources&& vsRes,
        ID3D11HullShader* hs, shader_resources&& hsRes,
        ID3D11DomainShader *ds, shader_resources&& dsRes,
        ID3D11PixelShader *ps, shader_resources&& psRes)
    : domainShader(ds), hullShader(hs), inputLayout(il), _name(name),
        pixelShader(ps), primitiveTopology(pt), vertexShader(vs) {
    this->_resources[shader_stage::vertex] = std::move(vsRes);
    this->_resources[shader_stage::hull] = std::move(hsRes);
    this->_resources[shader_stage::domain] = std::move(dsRes);
    this->_resources[shader_stage::pixel] = std::move(psRes);
}


/*
 * trrojan::d3d11::rendering_technique::apply
 */
void trrojan::d3d11::rendering_technique::apply(ID3D11DeviceContext *ctx) {
    assert(ctx != nullptr);

    /* Configure input. */
    ctx->IASetInputLayout(this->inputLayout);
    ctx->IASetPrimitiveTopology(this->primitiveTopology);
    {
        std::vector<ID3D11Buffer *> vbs;
        std::vector<UINT> offsets;
        std::vector<UINT> strides;

        vbs.reserve(this->vertexBuffers.size());
        offsets.reserve(this->vertexBuffers.size());
        strides.reserve(this->vertexBuffers.size());

        for (auto& v : this->vertexBuffers) {
            vbs.push_back(v.buffer.p);
            offsets.push_back(v.offset);
            strides.push_back(v.stride);
        }

        ctx->IASetVertexBuffers(0, static_cast<UINT>(vbs.size()), vbs.data(),
            strides.data(), offsets.data());
    }

    /* Configure shaders. */
    ctx->VSSetShader(this->vertexShader.p, nullptr, 0);
    ctx->DSSetShader(this->domainShader.p, nullptr, 0);
    ctx->HSSetShader(this->hullShader, nullptr, 0);
    ctx->GSSetShader(this->geometryShader, nullptr, 0);
    ctx->PSSetShader(this->pixelShader, nullptr, 0);

    /* Bind shader resources. */
    for (auto& res : this->_resources) {
        switch (res.first) {
            case shader_stage::vertex:
                if (this->vertexShader != nullptr) {
                    auto cbs = unsmart(res.second.constant_buffers);
                    ctx->VSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                        cbs.data());
                    auto srvs = unsmart(res.second.resource_views);
                    ctx->VSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                        srvs.data());
                    auto samplers = unsmart(res.second.sampler_states);
                    ctx->VSSetSamplers(0, static_cast<UINT>(samplers.size()),
                        samplers.data());
                }
                break;

            case shader_stage::hull:
                if (this->hullShader != nullptr) {
                    auto cbs = unsmart(res.second.constant_buffers);
                    ctx->HSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                        cbs.data());
                    auto srvs = unsmart(res.second.resource_views);
                    ctx->HSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                        srvs.data());
                    auto samplers = unsmart(res.second.sampler_states);
                    ctx->HSSetSamplers(0, static_cast<UINT>(samplers.size()),
                        samplers.data());
                }
                break;

            case shader_stage::domain:
                if (this->domainShader != nullptr) {
                    auto cbs = unsmart(res.second.constant_buffers);
                    ctx->DSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                        cbs.data());
                    auto srvs = unsmart(res.second.resource_views);
                    ctx->DSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                        srvs.data());
                    auto samplers = unsmart(res.second.sampler_states);
                    ctx->DSSetSamplers(0, static_cast<UINT>(samplers.size()),
                        samplers.data());
                }
                break;

            case shader_stage::geometry:
                if (this->geometryShader != nullptr) {
                    auto cbs = unsmart(res.second.constant_buffers);
                    ctx->GSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                        cbs.data());
                    auto srvs = unsmart(res.second.resource_views);
                    ctx->GSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                        srvs.data());
                    auto samplers = unsmart(res.second.sampler_states);
                    ctx->GSSetSamplers(0, static_cast<UINT>(samplers.size()),
                        samplers.data());
                }
                break;

            case shader_stage::pixel:
                if (this->pixelShader != nullptr) {
                    auto cbs = unsmart(res.second.constant_buffers);
                    ctx->PSSetConstantBuffers(0, static_cast<UINT>(cbs.size()),
                        cbs.data());
                    auto srvs = unsmart(res.second.resource_views);
                    ctx->PSSetShaderResources(0, static_cast<UINT>(srvs.size()),
                        srvs.data());
                    auto samplers = unsmart(res.second.sampler_states);
                    ctx->PSSetSamplers(0, static_cast<UINT>(samplers.size()),
                        samplers.data());
                }
                break;
        }
    }
}


/*
 * trrojan::d3d11::rendering_technique::set_constant_buffers
 */
void trrojan::d3d11::rendering_technique::set_constant_buffers(
        const std::vector<buffer_type>& buffers,
        const shader_stages stages, const UINT start) {
    this->foreach_stage(stages, [&buffers, &start](shader_resources& r) {
        auto& dst = r.constant_buffers;
        assert_range(dst, buffers, start);
        std::copy(buffers.begin(), buffers.end(), dst.begin() + start);
    });
}


/*
 * trrojan::d3d11::rendering_technique::set_shader_resource_views
 */
void trrojan::d3d11::rendering_technique::set_shader_resource_views(
        const std::vector<srv_type>& srvs,
        const shader_stages stages, const UINT start) {
    this->foreach_stage(stages, [&srvs, &start](shader_resources& r) {
        auto& dst = r.resource_views;
        assert_range(dst, srvs, start);
        std::copy(srvs.begin(), srvs.end(), dst.begin() + start);
    });
}


/*
 * trrojan::d3d11::rendering_technique::set_shader_resource_views
 */
void trrojan::d3d11::rendering_technique::set_shader_resource_views(
        const srv_type& srv, const shader_stages stages, const UINT start) {
    this->foreach_stage(stages, [&srv, &start](shader_resources& r) {
        auto& dst = r.resource_views;
        assert_range(dst, start);
        dst[start] = srv;
    });
}



/*
 * trrojan::d3d11::rendering_technique::set_vertex_buffers
 */
void trrojan::d3d11::rendering_technique::set_vertex_buffers(
        const std::vector<vertex_buffer>& vbs, const UINT start) {
    assert_range(this->vertexBuffers, vbs, start);
    std::copy(vbs.begin(), vbs.end(), this->vertexBuffers.begin() + start);
}


/*
 * trrojan::d3d11::rendering_technique::set_vertex_buffers
 */
void trrojan::d3d11::rendering_technique::set_vertex_buffers(
        const vertex_buffer& vb, const UINT start) {
    assert_range(this->vertexBuffers, start);
    this->vertexBuffers[start] = vb;
}


/*
 * trrojan::d3d11::rendering_technique::foreach_stage
 */
void trrojan::d3d11::rendering_technique::foreach_stage(
        const shader_stages stages,
        const std::function<void(shader_resources&)>& action) {
    for (auto s = static_cast<shader_stages>(shader_stage::vertex);
            s <= static_cast<shader_stages>(shader_stage::pixel);
            s <<= 1) {
        if ((s & stages) != 0) {
            action(this->_resources[static_cast<shader_stage>(s)]);
        }
    }
}
