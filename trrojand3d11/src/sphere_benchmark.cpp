/// <copyright file="sphere_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_benchmark.h"

#include <cassert>
#include <cinttypes>
#include <memory>

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/log.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/result.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/utilities.h"

#include "SphereGeometryShader.h"
#include "SpherePipeline.hlsli"
#include "SpherePixelShader.h"
#include "SphereVertexShader.h"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d11::sphere_benchmark::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(data_set);
_SPHERE_BENCH_DEFINE_FACTOR(frame);
_SPHERE_BENCH_DEFINE_FACTOR(method);

#undef _SPHERE_BENCH_DEFINE_FACTOR


#define _SPHERE_BENCH_DEFINE_METHOD(m)                                         \
const char *trrojan::d3d11::sphere_benchmark::method_##m = #m

_SPHERE_BENCH_DEFINE_METHOD(geosprite);
_SPHERE_BENCH_DEFINE_METHOD(tesssprite);
_SPHERE_BENCH_DEFINE_METHOD(tesssphere);
_SPHERE_BENCH_DEFINE_METHOD(tesshemisphere);

#undef _SPHERE_BENCH_DEFINE_METHOD


/*
 * trrojan::d3d11::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::sphere_benchmark(void)
        : benchmark_base("sphere-raycaster") {
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_frame, static_cast<frame_type>(0)));
}


/*
 * trrojan::d3d11::sphere_benchmark::~sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::~sphere_benchmark(void) { }


/*
 * trrojan::d3d11::sphere_benchmark::draw_debug_view
 */
void trrojan::d3d11::sphere_benchmark::draw_debug_view(
        ATL::CComPtr<ID3D11Device> device,
        ATL::CComPtr<ID3D11DeviceContext> deviceContext) {
    assert(device != nullptr);
    assert(deviceContext != nullptr);

    //deviceContext->ClearRenderTargetView()
}


/*
 * trrojan::d3d11::sphere_benchmark::on_debug_view_resized
 */
void trrojan::d3d11::sphere_benchmark::on_debug_view_resized(
        ATL::CComPtr<ID3D11Device> device,
        const unsigned int width, const unsigned int height) {
}


/*
 * trrojan::d3d11::sphere_benchmark::on_debug_view_resizing
 */
void trrojan::d3d11::sphere_benchmark::on_debug_view_resizing(void) {
}


/*
 * trrojan::d3d11::sphere_benchmark::optimise_order
 */
void trrojan::d3d11::sphere_benchmark::optimise_order(
        configuration_set& inOutConfs) {
    inOutConfs.optimise_order({ factor_device, factor_data_set });
}


/*
 * trrojan::d3d11::sphere_benchmark::required_factors
 */
std::vector<std::string> trrojan::d3d11::sphere_benchmark::required_factors(
        void) const {
    static const std::vector<std::string> retval = { factor_data_set,
        factor_device };
    return retval;
}


/*
 * trrojan::d3d11::sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d11::sphere_benchmark::on_run(d3d11::device& device,
        const configuration& config, const std::vector<std::string>& changed) {
    SphereConstants constants;
    auto ctx = device.d3d_context();
    auto dev = device.d3d_device();
    auto isNewDevice = contains(factor_device, changed);
    D3D11_VIEWPORT viewport;

    if (isNewDevice) {
        log::instance().write_line(log_level::verbose, "Preparing GPU "
            "resources for device \"%s\" ...", device.name().c_str());
        this->geometry_shader = create_geometry_shader(device.d3d_device(),
            ::SphereGeometryShaderBytes);
        this->pixel_shader = create_pixel_shader(device.d3d_device(),
            ::SpherePixelShaderBytes);
        this->vertex_shader = create_vertex_shader(device.d3d_device(),
            ::SphereVertexShaderBytes);
        this->constant_buffer = create_buffer(device.d3d_device(),
            D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, nullptr,
            static_cast<UINT>(sizeof(SphereConstants)));
    }

    if (contains(factor_data_set, changed)) {
        auto path = config.get<std::string>(factor_data_set);
        log::instance().write_line(log_level::verbose, "Loading MMPLD data set "
            "\"%s\" ...", path.c_str());
        this->open_mmpld(path.c_str());
    }

    if (isNewDevice || contains(factor_frame, changed)) {
        // Read the frame.
        auto frame = config.get<frame_type>(factor_frame);
        log::instance().write_line(log_level::verbose, "Loading MMPLD frame "
            "%u ...", frame);
        this->vertex_buffer = this->read_mmpld_frame(dev, frame);
        this->input_layout = create_input_layout(device.d3d_device(),
            this->mmpld_layout, ::SphereVertexShaderBytes);
    }

    {
        // Retrieve the viewport.
        auto vp = config.get<viewport_type>(factor_viewport);
        viewport.TopLeftX = viewport.TopLeftY = 0.0f;
        viewport.Width = vp[0];
        viewport.Height = vp[1];
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
    }

    /* Compute the matrices. */
    {
        auto projection = DirectX::XMMatrixPerspectiveFovRH(60.0f,
            static_cast<float>(viewport.Width) / static_cast<float>(viewport.Height),
            0.1f, 10.0f);
        auto view = DirectX::XMMatrixTranslation(0.0f, 0.0f, 3.0f);

        auto viewDet = DirectX::XMMatrixDeterminant(view);
        auto viewInv = DirectX::XMMatrixInverse(&viewDet, view);
        DirectX::XMStoreFloat4x4(&constants.ViewInvMatrix,
            DirectX::XMMatrixTranspose(viewInv));

        auto viewProj = view * projection;
        DirectX::XMStoreFloat4x4(&constants.ViewProjMatrix,
            DirectX::XMMatrixTranspose(viewProj));

        auto viewProjDet = DirectX::XMMatrixDeterminant(viewProj);
        auto viewProjInv = DirectX::XMMatrixInverse(&viewProjDet, viewProj);
        DirectX::XMStoreFloat4x4(&constants.ViewProjInvMatrix,
            DirectX::XMMatrixTranspose(viewProjInv));

        // TODO: read from device
        constants.Viewport.x = 0.0f;
        constants.Viewport.y = 0.0f;
        constants.Viewport.z = viewport.Width;
        constants.Viewport.w = viewport.Height;

        ctx->UpdateSubresource(this->constant_buffer.p, 0, nullptr,
            &constants, 0, 0);
    }

    /* Configure vertex stage. */
    ctx->VSSetShader(this->vertex_shader.p, nullptr, 0);

    /* Configure geometry stage. */
    ctx->GSSetShader(this->geometry_shader.p, nullptr, 0);
    ctx->GSSetConstantBuffers(0, 1, &this->constant_buffer.p);

    /* Configure pixel stage. */
    ctx->PSSetShader(this->pixel_shader.p, nullptr, 0);
    ctx->PSSetConstantBuffers(0, 1, &this->constant_buffer.p);

    /* Set vertex buffer. */
    const UINT offset = 0;
    const UINT stride = mmpld_reader::calc_stride(this->mmpld_list);
    ctx->IASetVertexBuffers(0, 1, &this->vertex_buffer.p, &stride, &offset);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
    ctx->IASetInputLayout(this->input_layout.p);

    /* Configure the rasteriser. */
    ctx->RSSetViewports(1, &viewport);
    //context->RSSetState(this->rasteriserState.Get());

    /* Render it. */
    assert(this->mmpld_list.particles <= UINT_MAX);
    ctx->Draw(static_cast<UINT>(this->mmpld_list.particles), 0);


    // TODO
    std::vector<std::string> results = { "todo" };
    auto retval = std::make_shared<basic_result>(
        config, std::move(results));

    retval->add({ 42 });

    return retval;
}
