/// <copyright file="sphere_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/sphere_benchmark.h"

#include <cassert>
#include <cinttypes>

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/mmpld_reader.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "SphereGeometryShader.h"
#include "SpherePixelShader.h"
#include "SphereVertexShader.h"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const std::string trrojan::d3d11::sphere_benchmark::factor_##f(#f)

_SPHERE_BENCH_DEFINE_FACTOR(data_set);

#undef _SPHERE_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d11::sphere_benchmark::sphere_benchmark
 */
trrojan::d3d11::sphere_benchmark::sphere_benchmark(void)
    : benchmark_base("sphere-raycaster") { }


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
 * trrojan::d3d11::sphere_benchmark::on_device_changed
 */
void trrojan::d3d11::sphere_benchmark::on_device_changed(d3d11::device& device,
        const configuration& config) {

}


/*
 * trrojan::d3d11::sphere_benchmark::on_run
 */
trrojan::result trrojan::d3d11::sphere_benchmark::on_run(d3d11::device& device,
        const configuration& config) {
    return result();
}
