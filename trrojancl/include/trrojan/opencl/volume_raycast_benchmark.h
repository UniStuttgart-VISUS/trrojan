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
#include "trrojan/opencl/util.h"

#include "trrojan/enum_parse_helper.h"

#include <unordered_set>
#include <unordered_map>

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

        // TODO remove hard coded paths
        static const std::string kernel_source_path;
        static const std::string kernel_snippet_path;
        static const std::string test_volume;

        // factor strings
        static const std::string factor_environment;
        static const std::string factor_device;

        static const std::string factor_iterations;
        static const std::string factor_volume_file_name;
        static const std::string factor_tff_file_name;
        static const std::string factor_viewport_width;
        static const std::string factor_viewport_height;
        static const std::string factor_step_size_factor;
        static const std::string factor_roll;
        static const std::string factor_pitch;
        static const std::string factor_yaw;
        static const std::string factor_zoom;

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

        enum kernel_arg
        {
            VOLUME = 0,     // volume data set      memory object
            OUTPUT,         // output image         memory object
            TFF,            // transfer function    memory object
            VIEW,           // view matrix          memory object
            ID,             // shuffled ray IDs     memory object
            STEP_SIZE,      // step size factor     cl_float
            RESOLUTION,     // volume resolution    cl_int3
            SAMPLER,        // image data sampler   cl::Sampler
            PRECISION,      // precision divisor    cl_float
            // OFFSET       // TODO: ID offset      cl_int2
        };

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
        virtual result run(const configuration &cfg);

    private:

        /// <summary>
        /// Add a factor that is relevant during kernel run-time.
        /// </summary>
        /// <param ="name">Name of the factor</param>
        /// <param name="value">Value of the factor</param>
        void add_kernel_run_factor(std::string name, variant value);

        /// <summary>
        /// Add a factor that is relevant during kernel build time.
        /// </summary>
        /// <param ="name">Name of the factor</param>
        /// <param name="value">Value of the factor</param>
        void add_kernel_build_factor(std::string name, variant value);

        /// <summary>
        /// Set-up basic raycaster configuration.
        /// </summary>
        /// <remarks>Normally, this method only needs to be invoked once
        /// before the first run.</remarks>
        /// <param name="cfg">The currently active configuration.</param>
        void setup_raycaster(const configuration &cfg);

        /// <summary>
        /// Setup the volume data set with the given configuration <paramref name="cfg" />.
        /// </summary>
        /// <param name="cfg">Refenrence to the configuration that is to be set-up.</param>
        /// <param name="changed">Set of factor names that have changed since the last run</param>
        void setup_volume_data(const configuration &cfg,
                               const std::unordered_set<std::string> changed);

        /// <summary>
        /// Load volume data based on information from the given .dat file.
        /// </summary>
        /// <param name="dat_file">Name of the .dat-file that contains the information
        /// on the volume data.</param>
        const std::vector<char> &load_volume_data(const std::string dat_file);

        /// <summary>
        /// Read a transfer function from the file with the given name.
        /// A transfer function has exactly 256 RGBA floating point values.
        /// We try to find those in the given input file by parsing for whitespace separated
        /// floating point values.
        /// However, if there are too many values in the file, we trunctuate respectively
        /// fill with zeros.
        /// If no trransfer function file is specified (i.e. the factor string is "fallback"),
        /// we use a default linear function with range [0;1] as fallback.
        /// </summary>
        /// <remarks>The read will fail on the first sign that is neither a numeric value,
        /// nor a whitespace</remarks>
        /// <param name="file_name">The name (and path) of the file that contains the
        /// transfer function in form of numeric values.</param>
        void load_transfer_function(const std::string file_name, environment::pointer env);

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
                this->dispatch<S>(scalar_type_list(), t, std::forward<P>(params)...);
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
                               t, std::forward<P>(params)...);
            }
        }


        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<trrojan::opencl::scalar_type S, class... P>
        inline void dispatch(trrojan::opencl::scalar_type_list_t<>,
                             const trrojan::opencl::scalar_type t,
                             P&&... params)
        {
            throw std::runtime_error("Resolution failed.");
        }


        /// <summary>
        /// Convert scalar raw volume data from a given input type to a given output type
        /// and create an OpenCL memory object with the resulting data.
        /// </summary>
        /// <param name="volume_data">Reference to the scalar input data</param>
        /// <param name="ue_buffer">Switch parameter to indicate whether a linear buffer
        /// or a 3d image buffer is to be created in OpenCL.</param>
        /// <tParam name="From">Data precision of the input scalar volume data.</tParam>
        /// <tParam name="To">Data precision of the data from which the OpenCL memory
        /// objects are to be created</tParam>
        template<class From, class To>
        void convert_data_precision(const std::vector<char> &volume_data,
                                    const bool use_buffer,
                                    environment::pointer cl_env)
        {
            // reinterpret raw data (char) to input format
            auto s = reinterpret_cast<const From *>(volume_data.data());
            auto e = reinterpret_cast<const From *>(volume_data.data() +
                                                    sizeof(From)*volume_data.size());

            // convert imput vector to the desired output precision
            std::vector<To> converted_data(s, e);
            try
            {
                if (use_buffer)
                {
                    _volume_mem = cl::Buffer(cl_env->get_properties().context,
                                             CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             converted_data.size()*sizeof(To),
                                             converted_data.data());
                }
                else    // texture
                {
                    cl::ImageFormat format;
                    format.image_channel_order = CL_R;
                    switch (sizeof(To))
                    {
                    case 1:
                        format.image_channel_data_type = CL_UNSIGNED_INT8; break;
                    case 2:
                        format.image_channel_data_type = CL_UNSIGNED_INT16; break;
                    case 4:
                        format.image_channel_data_type = CL_UNSIGNED_INT32; break;
                    case 8:
                        throw std::invalid_argument(
                                    "Double precision is not supported for OpenCL image formats.");
                        break;
                    default:
                        throw std::invalid_argument("Invalid volume data format."); break;
                    }

                    _volume_mem = cl::Image3D(cl_env->get_properties().context,
                                              CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                              format,
                                              _dr.properties().volume_res[0],
                                              _dr.properties().volume_res[1],
                                              _dr.properties().volume_res[2],
                                              0, 0,
                                              converted_data.data());
                }
            }
            catch (cl::Error err)
            {
                throw std::runtime_error( "ERROR: " + std::string(err.what()) + "("
                                          + util::get_cl_error_str(err.err()) + ")");
            }
            // Add memory object to manual OpenCL memory manager.
//            cl_env->get_garbage_collector().add_mem_object(&_volume_mem);
        }

        /// <summary>
        /// Parse a named variant for a scalar type.
        /// </summary>
        /// <param name="s">Refenrence to the named variant that is to be parsed.</param>
        /// <returns>The scalar type</returns>
        static inline scalar_type parse_scalar_type(const trrojan::named_variant& s)
        {
            typedef enum_parse_helper<scalar_type, scalar_type_traits, scalar_type_list_t> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(scalar_type_list(), value);
        }

        /// <summary>
        /// Create an OpenCL memory object.
        /// </summary>
        /// <param> TODO </param>
        void create_vol_mem(const scalar_type data_precision,
                           const scalar_type sample_precision,
                           const std::vector<char> &raw_data,
                           const bool use_buffer,
                           environment::pointer env);

        /// <summary>
        /// Compose and generate the OpenCL kernel source based on the given configuration.
        /// </summary>
        void compose_kernel(const configuration &cfg);

        /// <summary>
        /// Compile the OpenCL kernel source for the device referenced by <paramref name="dev" \>
        /// on the plattform referenced by <paramref name="env" \>.
        /// </summary>
        /// <param name="env">Smart pointer to a valid OpenCL environment.</param>
        /// <param name="dev">Smart pointer to a valid OpenCL device on the platform
        /// <paramref name="env" \>.</param>
        /// <param name="precision_div">Precision based devisor for kernel argument.</param>
        /// <param name="build_flags">Compiler build flags.</param>
        /// <throws>Runtime error if program creation, kernel build or initialization
        /// fail.</throws>
        void build_kernel(environment::pointer env,
                          device::pointer dev,
                          const float precision_div = 255.0f,
                          const std::string build_flags = "");

        /// <summary>
        /// Set all constant kernel arguments such as the OpenCL memory objects.
        /// </summary>
        void set_kernel_args(const float precision_div);

        /// <summary>
        /// Update arguments that are relavant for kernel execution during runtime.
        /// </summary>
        /// <param name="cfg">The current configuration.</param>
        /// <param name="changed">List of all configuration parameter names that have changed
        /// since the last run.</param>
        void update_kernel_args(const configuration &cfg,
                                const std::unordered_set<std::string> changed);

        /// <summary>
        /// Read all kernel snippets that can be found in <paramref name="path" />,
        /// i.e. all files with the ".cl" extension.
        /// </summay>
        /// <param name="path">The directory path containing the kernel snippets.</param>
        void read_kernel_snippets(const std::string path);

        /// <summary>
        /// Replace the first keyword <paramref name="keyword" /> string that can be found in
        /// <paramref name="text" /> with the <paramref name="insert" /> string. Keywords
        /// in the <paramref name="text" /> have to be surrounded by <paramref name="prefix" />
        /// and <paramref name="suffix" /> , the defauls are /***keyword***/.
        /// <param name="keyword">The keyword string that is to be replaced</param>
        /// <param name="insert">The string that is to be inserted in place of keyword</param>
        /// <param name="text">Reference to the string that os to be manipulated.</param>
        /// <param name="prefix">Keyword defining prefix, default is "/***".</param>
        /// <param name="suffix">Keyword defining suffix, default is "***/".</param>
        void replace_keyword(const std::string keyword,
                             const std::string insert,
                             std::string &text,
                             const std::string prefix = "/***",
                             const std::string suffix = "***/");

        /// <summary>
        /// Replace a keyword in <paramref name="kernel_source" /> with the snipped contained
        /// in the _kernel_snippets member map, accessed by the <paramref name="keyword" />.
        /// <param name="keyword">The keyword used as the key to access the snippet in
        /// _kernel_snippets member variable.</param>
        /// <param name="kernel_source">Reference to the kernel source that is to be
        /// manipulated.</param>
        void replace_kernel_snippet(const std::string keyword, std::string &kernel_source);

        /// <summary>
        /// Create a right handed, transposed view matrix from <paramref name="roll" />,
        /// <paramref name="pitch" />, <paramref name="yaw" /> rotatians as well as the
        /// camera distance (<paramref name="zoom />").
        /// <param name="yaw">Rotation around the y-axis in radians.</param>
        /// <param name="pitch">Rotation around the x-axis in radians.</param>
        /// <param name="roll">Rotation around the z-axis in radians.</param>
        /// <param name="zoom">Distance of the camera from the origin
        /// (where the volume is centered)</param>
        /// <remarks>Right handed coordinate system.</remarks>
        /// <remarks>Assuming radians as input angles.</remarks>
        /// <returns>The updated RH view matrix.</returns>
        std::array<float, 16> create_view_mat(double roll, double pitch, double yaw, double zoom);

        /// <summary>
        /// Interpret an OpenCL error <paramref name="error" /> and throw.
        /// TODO: log to file.
        /// </summary>
        /// <param name="error">The OpenCL error objects.</param>
        /// <throws>Runtime error.</throws>
        void log_cl_error(cl::Error error);

        /// <summary>
        /// Member to hold 'passive' configuration factors (i.e. they have no influence on tests),
        /// that are read from the volume data set ".dat" file.
        /// </summary>
        trrojan::configuration _passive_cfg;

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

        /// <summary>
        /// Unordered map to store OpenCL kernel snippets.
        /// </summary>
        std::unordered_map<std::string, std::string> _kernel_snippets;

        /// <summary>
        /// Volume data as OpenCL memory object.
        /// </summary>
        /// <remarks>Can be represented either as a linear buffer or as a 3d image object.
        cl::Memory _volume_mem;

        /// <summary>
        /// The rendering output image.
        /// </summary>
        cl::Image2D _output_mem;

        /// <summary>
        /// Transfer function memory object as a 1d image representation.
        /// </summary>
        cl::Image1D _tff_mem;

        /// <summary>
        /// OpenCL buffer object for view matrix.
        /// </summary>
        cl::Buffer _view_mat;

        /// <summary>
        /// OpenCL buffer object for suffled ray IDs.
        /// </summary>
        cl::Buffer _ray_ids;

        /// <summary>
        /// Sampler for images in OpenCL kernel.
        /// </summary>
        cl::Sampler _sampler;

        /// <summary>
        /// The current OpenCL kernel for volume raycasting.
        /// </summary>
        cl::Kernel _kernel;

        /// <summary>
        /// Complete source of the current OpenCL kernel.
        /// </summary>
        std::string _kernel_source;

        /// <summary>
        /// Vector for storing the rendered output data (2d image).
        /// </summary>
        std::vector<float> _output_data;
    };

}
}
