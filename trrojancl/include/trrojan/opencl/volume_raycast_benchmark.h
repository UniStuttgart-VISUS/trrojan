/// <copyright file="volume_raycast_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/benchmark.h"

#include "trrojan/opencl/export.h"


namespace trrojan
{
namespace opencl
{

    /// <summary>
    /// The implementation of a basic volume raycasting benchmark.
    /// </summary>
    /// <remarks>
    /// The implementation of this benchmark is closely related to the volume renderer
    /// from the NVIDIA OpenCL SDK:
    /// http://developer.download.nvidia.com/compute/cuda/3_0/sdk/website/OpenCL/website/samples.html#oclVolumeRender
    /// However, instead of using back-to-front compositing, rays are cast in
    /// front-to-back order, which also enables the use of early ray termination (ERT).
    /// </remarks>
    class TRROJANCL_API volume_raycast_benchmark : public trrojan::benchmark_base
    {

    public:

        inline volume_raycast_benchmark(void) : trrojan::benchmark_base("volume_raycast") { }

        /// <summary>
        /// Destructor.
        /// </summary>
        virtual ~volume_raycast_benchmark(void);

        /// <summary>
        /// Overrides benchmark run method.
        /// </summary>
        virtual result_set run(const configuration_set& configs);

    private:
        /// <summary>
        /// Setup the raycaster with the given configuration.
        /// </summary>
        void setup_raycaster(const configuration_set& configs);

        /// <summary>
        /// Compose and generate the OpenCL kernel source based on the given configuration.
        /// </summary>
        void compose_kernel(const configuration_set& configs);

        /// <summary>
        /// Compile the OpenCL kernel source.
        /// </summary>
        void build_kernel();

        /// <summary>
        /// Run the OpenCL kernel.
        /// </summary>
        void run_kernel();
    };

}
}
