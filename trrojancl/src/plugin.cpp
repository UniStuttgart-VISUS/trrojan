/// <copyright file="plugin.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/plugin.h"

#include "trrojan/opencl/environment.h"

#include "trrojan/opencl/volume_raycast_benchmark.h"


/// <summary>
/// Gets a new instance of the plugin descriptor.
/// </summary>
extern "C" TRROJANCL_API trrojan::plugin_base *get_trrojan_plugin(void)
{
    return new trrojan::opencl::plugin();
}


/*
 * trrojan::opencl::plugin::~plugin
 */
trrojan::opencl::plugin::~plugin(void) { }


/**
 * trrojan::opencl::plugin::create_benchmarks
 */
size_t trrojan::opencl::plugin::create_benchmarks(benchmark_list& dst) const
{
    dst.push_back(std::make_shared<volume_raycast_benchmark>());
    return 1;
}


/*
 * trrojan::opencl::plugin::create_environments
 */
size_t trrojan::opencl::plugin::create_environments(environment_list& dst) const
{
    // create one environment per OpenCL platform that is found
    cmd_line l;
    std::vector<std::string> names;
    size_t platform_cnt = environment::get_platform_names(names);
    for (size_t i = 0; i < platform_cnt; ++i)
    {
        environment e(names.at(i));
        e.on_initialize(l, i);
        dst.push_back(std::make_shared<environment>(e));
    }
    return platform_cnt;
}
