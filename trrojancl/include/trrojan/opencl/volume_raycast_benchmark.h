/// <copyright file="volume_raycast_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#pragma once

#include "trrojan/benchmark.h"

#include "trrojan/opencl/export.h"
#include "trrojan/opencl/scalar_type.h"
#include "trrojan/opencl/dat_raw_reader.h"
#include "trrojan/opencl/environment.h"

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

        static const std::string factor_iterations;
        static const std::string factor_volume_file_name;
        static const std::string factor_tff_file_name;
        static const std::string factor_viewport_width;
        static const std::string factor_viewport_height;
        static const std::string factor_step_size_factor;
        static const std::string factor_view_rot_x;
        static const std::string factor_view_rot_y;
        static const std::string factor_view_rot_z;
        static const std::string factor_view_pos_x;
        static const std::string factor_view_pos_y;
        static const std::string factor_view_pos_z;

        static const std::string factor_sample_precision;
        static const std::string factor_use_lerp;
        static const std::string factor_use_ERT;
        static const std::string factor_use_tff;
        static const std::string factor_use_dvr;
        static const std::string factor_shuffle;
        static const std::string factor_use_buffer;
        static const std::string factor_use_illumination;
        static const std::string factor_use_ortho_proj;

        static const std::string factor_img_output;
        static const std::string factor_count_samples;

        static const std::string factor_data_precision;
        static const std::string factor_volume_res_x;
        static const std::string factor_volume_res_y;
        static const std::string factor_volume_res_z;

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
        const std::vector<char> &load_volume_data(const std::string dat_file,
                                                  configuration &static_cfg);

        /// <summary>
        /// Selects the correct source scalar type <paramref name="s" />
        /// and continues with dispatching the target type.
        /// </summary>
        template<trrojan::opencl::scalar_type S,
            trrojan::opencl::scalar_type... Ss,
            class... P>
        inline void dispatch(
                trrojan::opencl::scalar_type_list_t<S, Ss...>,
                const trrojan::opencl::scalar_type s,
                const trrojan::opencl::scalar_type t,
                P&&... params)
        {
            if (S == s)
            {
                //std::cout << "scalar type " << (int) S << " selected." << std::endl;
                this->dispatch<S>(scalar_type_list(), s, t, std::forward<P>(params)...);
            }
            else
            {
                this->dispatch(trrojan::opencl::scalar_type_list_t<Ss...>(),
                               s, t, std::forward<P>(params)...);
            }
        }


        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<class... P>
        inline void dispatch(trrojan::opencl::scalar_type_list_t<>,
                             const trrojan::opencl::scalar_type s,
                             const trrojan::opencl::scalar_type t,
                             P&&... params)
        {
            throw std::runtime_error("Resolution failed.");
        }


        /// <summary>
        /// Selects the specified target scalar type <paramref name="t" />
        /// and continues with the conversion.
        /// </summary>
        template<trrojan::opencl::scalar_type S,
            trrojan::opencl::scalar_type T,
            trrojan::opencl::scalar_type... Ts,
            class... P>
        inline void dispatch(
                trrojan::opencl::scalar_type_list_t<T, Ts...>,
                const trrojan::opencl::scalar_type s,
                const trrojan::opencl::scalar_type t,
                P&&... params)
        {
            if (T == t)
            {
                typedef typename scalar_type_traits<S>::type src_type;
                typedef typename scalar_type_traits<T>::type dst_type;
                this->convert_data_precision<src_type, dst_type>(
                            std::forward<P>(params)...);
            }
            else
            {
                this->dispatch<S>(trrojan::opencl::scalar_type_list_t<Ts...>(),
                               s, t, std::forward<P>(params)...);
            }
        }


        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<trrojan::opencl::scalar_type S, class... P>
        inline void dispatch(trrojan::opencl::scalar_type_list_t<>,
                             const trrojan::opencl::scalar_type s,
                             const trrojan::opencl::scalar_type t,
                             P&&... params)
        {
            throw std::runtime_error("Resolution failed.");
        }


        /// <summary>
        /// Convert scalar raw data.
        /// </summary>
        ///
        template<class From, class To>
        void convert_data_precision(const std::vector<char> &volume_data, const bool use_buffer)
        {
            // reinterpret raw data (char) to input format
            auto s = reinterpret_cast<const From *>(volume_data.data());
            auto e = reinterpret_cast<const From *>(volume_data.data() + volume_data.size());

            // convert imput vector to the desired output precision
            std::vector<To> converted_data(s, e);

            // TODO
            try
            {
                cl_int err = CL_SUCCESS;
                if (use_buffer)
                {
                    _vol_data = cl::Buffer(_env->get_properties().context,
                                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                converted_data.size(),
                                                converted_data.data(),
                                                &err);
                }
                else    // texture
                {
                    cl::ImageFormat format;
                    format.image_channel_order = CL_R;

                    format.image_channel_data_type = CL_UNORM_INT8;
                    format.image_channel_data_type = CL_UNORM_INT16;
                    format.image_channel_data_type = CL_FLOAT;

                    _vol_data = cl::Image3D(_env->get_properties().context,
                                        CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                        format,
                                        _dr.properties().volume_res[0],
                                        _dr.properties().volume_res[1],
                                        _dr.properties().volume_res[2],
                                        0, 0,
                                        converted_data.data(),
                                        &err);
                }
            }
            catch (cl::Error err)
            {
                throw std::runtime_error( "ERROR: " + std::string(err.what()) + "("
                                          + std::to_string(err.err()) + ")");
            }
            // Add memory object to manual OpenCL memory manager.
            _env->get_properties().gc.add_mem_object(&_vol_data);
        }

        ///
        /// \brief create_cl_mem
        ///
        void create_cl_mem(const scalar_type data_precision,
                           const scalar_type sample_precision,
                           const std::vector<char> &raw_data,
                           const bool use_buffer);

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

        /// <summary>
        /// Dat raw reader object;
        /// </summary>
        dat_raw_reader _dr;

        std::shared_ptr<trrojan::opencl::environment> _env;

        /// <summary>
        /// Volume data as OpenCL memory object.
        /// </summary>
        /// <remarks>Can be represented either as a linear buffer or as a 3d image object.
        cl::Memory _vol_data;
    };

}
}
