/// <copyright file="plugin.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/plugin.h"

#include "trrojan/stream/stream_benchmark.h"


/// <summary>
/// Gets a new instance of the plugin descriptor.
/// </summary>
extern "C" TRROJANSTREAM_API trrojan::plugin_base *get_trrojan_plugin(void) {
    return new trrojan::stream::plugin();
}


/*
 * trrojan::stream::plugin::~plugin
 */
trrojan::stream::plugin::~plugin(void) { }


/*
 * trrojan::stream::plugin::create_benchmarks
 */
size_t trrojan::stream::plugin::create_benchmarks(benchmark_list& dst) const {
    dst.push_back(std::make_shared<stream_benchmark>());
    return 1;
}


/*
 * trrojan::stream::plugin::create_environments
 */
size_t trrojan::stream::plugin::create_environments(
        environment_list& dst) const {
    return 0;
}
