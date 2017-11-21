/// <copyright file="plugin.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/plugin.h"

#include "trrojan/d3d11/environment.h"
#include "trrojan/d3d11/sphere_benchmark.h"


/// <summary>
/// Gets a new instance of the plugin descriptor.
/// </summary>
extern "C" TRROJAND3D11_API trrojan::plugin_base *get_trrojan_plugin(void) {
    return new trrojan::d3d11::plugin();
}


/*
 * trrojan::d3d11::plugin::~plugin
 */
trrojan::d3d11::plugin::~plugin(void) { }


/*
 * trrojan::d3d11::plugin::create_benchmarks
 */
size_t trrojan::d3d11::plugin::create_benchmarks(benchmark_list& dst) const {
    dst.emplace_back(std::make_shared<sphere_benchmark>());
    return 1;
}


/*
 * trrojan::d3d11::plugin::create_environments
 */
size_t trrojan::d3d11::plugin::create_environments(
        environment_list& dst) const {
    dst.emplace_back(std::make_shared<environment>());
    return 1;
}
