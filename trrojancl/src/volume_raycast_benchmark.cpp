/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"

/*
 *
 */
trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark(void) { }


/*
 *
 */
trrojan::result_set trrojan::opencl::volume_raycast_benchmark::run(
        const configuration_set& configs)
{
    this->check_required_factors(configs);

    // setup config
    setup_raycaster(configs);

    // compose CL kernel source
    compose_kernel(configs);

    // build kernel file
    build_kernel();

    // run CL kernel
//    for (int i = 0; i < /* # iterations factor */; ++i)
    {
        run_kernel();
    }

    return result_set();
}


/*
 * setup volume raycaster
 */
void trrojan::opencl::volume_raycast_benchmark::setup_raycaster(
        const trrojan::configuration_set &configs)
{

}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(
        const trrojan::configuration_set &configs)
{

}


/*
 * trrojan::opencl::volume_raycast_benchmark::generate_kernel
 */
void trrojan::opencl::volume_raycast_benchmark::build_kernel()
{

}

/**
 * trrojan::opencl::volume_raycast_benchmark::run_kernel
 */
void trrojan::opencl::volume_raycast_benchmark::run_kernel()
{

}
