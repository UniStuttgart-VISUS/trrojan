/// <copyright file="plugin.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/opencl/plugin.h"

#include "trrojan/opencl/environment.h"


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
    //dst.push_back(std::make_shared<opencl_benchmark>());
    return 1;
}


/*
 * trrojan::opencl::plugin::create_environments
 */
size_t trrojan::opencl::plugin::create_environments(environment_list& dst) const
{
    // create one environment per OpenCL platform that is found
    cmd_line l;
    for (size_t i = 0; i < environment::get_platform_cnt(); ++i)
    {
        environment e;
        e.on_initialize(l, i);
        dst.push_back(std::make_shared<environment>(e));
    }
    return environment::get_platform_cnt();
}
