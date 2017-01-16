/// <copyright file="volume_raycast_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/benchmark.h"

#include "trrojan/opencl/export.h"

#include "omp.h"

#ifdef _WIN32
    #include <unordered_set>
#else
    #include <tr1/unordered_set>
#endif

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

        typedef benchmark_base::on_result_callback on_result_callback;

        /// <summary>
        /// Constructor. Default config is defined here.
        /// </summary>
        volume_raycast_benchmark(void);

        /// <summary>
        /// Destructor.
        /// </summary>
        virtual ~volume_raycast_benchmark(void);

        /// <summary>
        /// Overrides benchmark run method.
        /// </summary>
        virtual size_t run(const configuration_set &configs,
            const on_result_callback& callback);

        /// <summary>
        /// Overrides benchmark run method.
        /// </summary>
        virtual result run(const configuration &configs);

    private:
        /// <summary>
        /// Setup the raycaster with the given configuration.
        /// </summary>
        void setup_raycaster(const configuration &cfg,
                             const std::tr1::unordered_set<std::string> changed);

        /// <summary>
        /// Load volume data based on information from the given .dat file.
        /// </summary>
        /// <param name="dat_file">Name of the .dat-file that contains the information
        /// on the volume data.</param>
        configuration load_volume_data(const std::string dat_file,
                                       const unsigned int sample_precision);


        ///
        /// deep copy TODO: performance
        ///
        template<class From, class To>
        const std::vector<To> convert_data_precision(const std::vector<char> &volume_data,
                                                     const size_t voxel_cnt,
                                                     unsigned int out_precision) const
        {
            unsigned short voxel_val = 0;

            if (volume_data.size() <= voxel_cnt * sizeof(From))
            {
                throw std::invalid_argument(
                            "Volume data size must correspond to the number of voxels. ");
            }

            std::vector<To> converted_data(voxel_cnt);

            for(size_t i = 0; i < voxel_cnt; ++i)
            {
                voxel_val = ((From*)(volume_data.data()))[i];

                if ((sizeof(From) == 1 && out_precision == 1)
                        || (sizeof(From) == 2 && out_precision == 2))
                {
                    converted_data.at(i) = voxel_val;
                }
                else if (sizeof(From) == 1 && out_precision == 2)
                {
                    converted_data.at(i) = voxel_val*256;
                }
                else if (sizeof(From) == 2 && out_precision == 1)
                {
                    converted_data.at(i) = voxel_val/256;
                }
                else if (out_precision == 4 || out_precision == 8)
                {
                    converted_data.at(i) =
                            voxel_val / (double)std::numeric_limits<From>::max();
                }
                else
                {
                    throw std::invalid_argument(
                                "Precision must be 1, 2, 4 or 8 bytes but is "
                                + std::to_string(out_precision) + " bytes");
                }
            }

            std::cout << "Converted data to " << out_precision << " byte(s)." << std::endl;
            return converted_data;
        }

        /// <summary>
        /// Compose and generate the OpenCL kernel source based on the given configuration.
        /// </summary>
        void compose_kernel(const configuration &cfg,
                            const std::tr1::unordered_set<std::string> changed);

        /// <summary>
        /// Compile the OpenCL kernel source.
        /// </summary>
        void build_kernel();

        /// <summary>
        /// Update runtime kernel arguments.
        /// </summary>
        /// \param cfg
        /// \param changed
        void update_kernel_args(const configuration &cfg,
                                const std::tr1::unordered_set<std::string> changed);

        /// <summary>
        /// Vector containing the names of all factors that are relevent at build time
        /// of the OpenCL kernel.
        /// </summary>
        std::vector<std::string> _kernel_build_factors;

        /// <summary>
        /// Vector containing the names of all factors that are relevent at run-time
        /// of the OpenCL kernel.
        /// </summary>
        std::vector<std::string> _kernel_run_factors;
    };

}
}
