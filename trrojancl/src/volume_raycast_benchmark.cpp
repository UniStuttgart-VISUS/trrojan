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
    auto cs = configs;

    /* _default_configs:
     *
     * - uint: # test repetitions                   5
     *
     * // Volume/view properties
     * - string: data set ".dat" file               <required>
     *      - uint3: volume resolution uint
     *      - uint: data precision
     * - uint2: viewport                            1024²
     * - uint: sample precision                     1
     * - double: step size factor                   0.5
     * - float4[8? 64?]: tff -> from file?          "grayscale"
     * - double3: view rotation                     0,0,0
     * - double3: view position                     0,0,-2
     *
     * // rendering modes
     * - bool: shuffle                              false
     * - bool: buffer / texture                     tex
     * - bool: nearest / linear interpolation       linear
     * - bool: illumination                         false
     * - bool: ERT                                  true
     * - bool: ortho / perspective                  perspective
     * - bool: tff lookup                           true
     * - bool: iso surface / tff                    tff
     *
     * // debug / misc bools                        false
     * - image output
     * - sample count
     * - memory pattern test
     *      - uint2 offset                          0,0
     */


    // TODO: only update delta to last config
    setup_raycaster(cs);

    compose_kernel(cs);

    build_kernel();

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
