/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"

#include <cassert>

#include "trrojan/timer.h"


#define _TRROJANSTREAM_DEFINE_FACTOR(f)                                        \
const std::string trrojan::opencl::volume_raycast_benchmark::factor_##f(#f)

_TRROJANSTREAM_DEFINE_FACTOR(environment);
_TRROJANSTREAM_DEFINE_FACTOR(device);

_TRROJANSTREAM_DEFINE_FACTOR(iterations);
_TRROJANSTREAM_DEFINE_FACTOR(volume_file_name);
_TRROJANSTREAM_DEFINE_FACTOR(tff_file_name);
_TRROJANSTREAM_DEFINE_FACTOR(viewport_width);
_TRROJANSTREAM_DEFINE_FACTOR(viewport_height);
_TRROJANSTREAM_DEFINE_FACTOR(step_size_factor);
_TRROJANSTREAM_DEFINE_FACTOR(view_rot_x);
_TRROJANSTREAM_DEFINE_FACTOR(view_rot_y);
_TRROJANSTREAM_DEFINE_FACTOR(view_rot_z);
_TRROJANSTREAM_DEFINE_FACTOR(view_pos_x);
_TRROJANSTREAM_DEFINE_FACTOR(view_pos_y);
_TRROJANSTREAM_DEFINE_FACTOR(view_pos_z);

_TRROJANSTREAM_DEFINE_FACTOR(sample_precision);
_TRROJANSTREAM_DEFINE_FACTOR(use_lerp);
_TRROJANSTREAM_DEFINE_FACTOR(use_ERT);
_TRROJANSTREAM_DEFINE_FACTOR(use_tff);
_TRROJANSTREAM_DEFINE_FACTOR(use_dvr);
_TRROJANSTREAM_DEFINE_FACTOR(shuffle);
_TRROJANSTREAM_DEFINE_FACTOR(use_buffer);
_TRROJANSTREAM_DEFINE_FACTOR(use_illumination);
_TRROJANSTREAM_DEFINE_FACTOR(use_ortho_proj);
_TRROJANSTREAM_DEFINE_FACTOR(img_output);
_TRROJANSTREAM_DEFINE_FACTOR(count_samples);

_TRROJANSTREAM_DEFINE_FACTOR(data_precision);
_TRROJANSTREAM_DEFINE_FACTOR(volume_res_x);
_TRROJANSTREAM_DEFINE_FACTOR(volume_res_y);
_TRROJANSTREAM_DEFINE_FACTOR(volume_res_z);

#undef _TRROJANSTREAM_DEFINE_FACTOR


/*
 * trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark(void)
    : trrojan::benchmark_base("volume_raycast")
{
    // default config
    //
    // TODO: @christoph empty factors (aka required factor)
//    this->_default_configs.add_factor(factor::empty("environment"));
//    this->_default_configs.add_factor(factor::empty("device"));

    // if no number of test iterations is specified, use a magic number
    this->_default_configs.add_factor(factor::from_manifestations(factor_iterations, 5));
    // volume and view properties -> basic config
    //
    // volume .dat file name is a required factor
//    this->_default_configs.add_factor(factor::empty("volume_file_name"));
    this->_default_configs.add_factor(factor::from_manifestations(
                                          factor_volume_file_name,
                                          std::string(
                                              "/media/brudervn/Daten/volTest/vol/bonsai.dat")));
    // transfer function file name, use a provided linear transfer function file as default
    this->_default_configs.add_factor(factor::from_manifestations(factor_tff_file_name,
                                                                  std::string("fallback")));

    // camera setup -> kernel runtime factors
    //
    add_kernel_run_factor(factor_viewport_width, 1024);
    add_kernel_run_factor(factor_viewport_height, 1024);
    add_kernel_run_factor(factor_step_size_factor, 0.5);
    add_kernel_run_factor(factor_view_rot_x, 0.0);
    add_kernel_run_factor(factor_view_rot_y, 0.0);
    add_kernel_run_factor(factor_view_rot_z, 0.0);
    add_kernel_run_factor(factor_view_pos_x, 0.0);
    add_kernel_run_factor(factor_view_pos_y, 0.0);
    add_kernel_run_factor(factor_view_pos_z, -2.0);

    // rendering modes -> kernel build factors
    //
    // sample precision in bytes, if not specified, use uchar (1 byte)
    add_kernel_build_factor(factor_sample_precision,
                            scalar_type_traits<scalar_type::uchar>::name());
    // use linear interpolation (not nearest neighbor interpolation)
    add_kernel_build_factor(factor_use_lerp, true);
    // use early ray termination
    add_kernel_build_factor(factor_use_ERT, true);
    // make a transfer function lookups
    add_kernel_build_factor(factor_use_tff, true);
    // use direct volume rendering (not inderect aka iso-surface rendering)
    add_kernel_build_factor(factor_use_dvr, true);
    // shuffle ray IDs pseudo randomly
    add_kernel_build_factor(factor_shuffle, false);
    // use a linear buffer as volume data structure (instead of a texture)
    add_kernel_build_factor(factor_use_buffer, false);
    // use a simple illumination technique
    add_kernel_build_factor(factor_use_illumination, false);
    // use an orthographic camera projection (instead of a perspective one)
    add_kernel_build_factor(factor_use_ortho_proj, false);

    // debug and misc testing configurations
    //
    // output a rendered image to file (PNG) -> basic config
    this->_default_configs.add_factor(factor::from_manifestations(factor_img_output, false));
    // count all samples taken along rays -> kernel build factor
    add_kernel_build_factor(factor_count_samples, false);

    // TODO: parameters for memory pattern test
    // perform a test for memory patterns
    // add_kernel_build_factor(factor_test_memory_patterns, false);
    // pixel offset
    // add_kernel_run_factor(factor_offset_x, 0);
    // add_kernel_run_factor(factor_offset_y, 0);
}

/*
 * trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark(void)
{
}


void trrojan::opencl::volume_raycast_benchmark::add_kernel_run_factor(std::string name,
                                                                      variant value)
{
    this->_default_configs.add_factor(factor::from_manifestations(name, value));
    this->_kernel_run_factors.push_back(name);
}


void trrojan::opencl::volume_raycast_benchmark::add_kernel_build_factor(std::string name,
                                                                        variant value)
{
    this->_default_configs.add_factor(factor::from_manifestations(name, value));
    this->_kernel_build_factors.push_back(name);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::run
 */
size_t trrojan::opencl::volume_raycast_benchmark::run(
        const configuration_set& configs, const on_result_callback& callback)
{
    std::unordered_set<std::string> changed;
    size_t retval;

    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto cs = configs;
    cs.merge(this->_default_configs, false);

    cs.foreach_configuration([&](const trrojan::configuration& cs)
    {
        changed.clear();
        this->check_changed_factors(cs, std::inserter(changed, changed.begin())); // TODO after static config merge
        for (auto& f : cs)
        {
            std::cout << f << std::endl;
        }
        std::cout << std::endl;

        // change the setup according to changed factors that are relevant
        setup_raycaster(cs, changed);

        // compose the OpenCL kernel according to the changed factors,
        // if at least one relevant factor changed
        if (std::any_of(_kernel_build_factors.begin(), _kernel_build_factors.end(),
                        [&](std::string s){return changed.count(s);}))
        {
            compose_kernel(cs, changed);
            // build the kernel file
            build_kernel();
        }

        // update the OpenCL kernel arguments according to the changed factors,
        // if at least one relevant factor changed
        if (std::any_of(_kernel_run_factors.begin(), _kernel_run_factors.end(),
                         [&](std::string s){return changed.count(s);}))
        {
            update_kernel_args(cs, changed);
        }


        // run the OpenCL kernel, i.e. the actual test
        auto r = callback(this->run(cs));
        ++retval;
        return r;
    });

    return retval;
}


/*
 * trrojan::opencl::volume_raycast_benchmark::run
 */
trrojan::result trrojan::opencl::volume_raycast_benchmark::run(const configuration &configs)
{


    // TODO measure kernel runtime and put that into result
    return trrojan::result();
}


/*
 * setup volume raycaster
 */
void trrojan::opencl::volume_raycast_benchmark::setup_raycaster(
        const trrojan::configuration &cfg,
        const std::unordered_set<std::string> changed)
{
    // contains the static factors that are defined through the volume data set ".dat" file
    trrojan::configuration static_cfg;
    std::vector<char> raw_data;

    // load volume data from dat-raw-file
    if (changed.count(factor_volume_file_name))
    {
        raw_data = load_volume_data(cfg.find(factor_volume_file_name)->value(), static_cfg);
    }

    auto env = cfg.find(factor_environment)->value().as<trrojan::environment>();

    // create OpenCL volume data memory object (either texture or linear buffer)
    if (changed.count(factor_sample_precision) || !raw_data.empty())
    {
        auto data_precision = parse_scalar_type(*static_cfg.find(factor_data_precision));
        auto sample_precision = parse_scalar_type(*cfg.find(factor_sample_precision));

        create_vol_mem(data_precision,
                      sample_precision,
                      raw_data,
                      cfg.find(factor_use_buffer)->value(),
                      std::dynamic_pointer_cast<environment>(env));
    }

    if (changed.count(factor_tff_file_name))
    {
        load_transfer_function(cfg.find(factor_tff_file_name)->value(),
                               std::dynamic_pointer_cast<environment>(env));
    }
}


/**
 * trrojan::opencl::volume_raycast_benchmark::load_volume_data
 */
const std::vector<char> & trrojan::opencl::volume_raycast_benchmark::load_volume_data(
        const std::string dat_file,
        trrojan::configuration &static_cfg)
{
    std::cout << "Loading volume data defined in " << dat_file << std::endl;

    try
    {
        _dr.read_files(dat_file);
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl;
        throw e;
    }

    std::cout << _dr.data().size() << " bytes have been read." << std::endl;
    std::cout << _dr.properties().to_string() << std::endl;

    if (_dr.properties().format == "UCHAR")
    {
        static_cfg.add(named_variant(factor_data_precision,
                                     scalar_type_traits<scalar_type::uchar>::name()));
    }
    else if (_dr.properties().format == "USHORT")
    {
        static_cfg.add(named_variant(factor_data_precision,
                                     scalar_type_traits<scalar_type::ushort>::name()));
    }
    else
    {
        throw std::invalid_argument("Unsupported volume data format defined in dat file.");
    }

    static_cfg.add(named_variant(factor_volume_res_x, _dr.properties().volume_res[0]));
    static_cfg.add(named_variant(factor_volume_res_y, _dr.properties().volume_res[1]));
    static_cfg.add(named_variant(factor_volume_res_z, _dr.properties().volume_res[2]));

    return _dr.data();
}

/**
 * @brief trrojan::opencl::volume_raycast_benchmark::load_transfer_function
 */
void trrojan::opencl::volume_raycast_benchmark::load_transfer_function(
        const std::string file_name,
        environment::pointer env)
{
    // The size of the transfer function vector (256 * RGBA values).
    size_t num_values = 256;
    std::vector<float> values;

    if (file_name == "fallback")
    {
        std::cerr << "WARNING: No transfer function defined, falling back to default: "
                     "linear function in range [0;1]." << std::endl;

        for (size_t i = 0; i < num_values; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                values.push_back(i * (1.0f / static_cast<float>(num_values - 1.0f)));
            }
        }
    }
    else    // try to read file
    {
        std::cout << "Loading transfer funtion data defined in " << file_name << std::endl;

        std::ifstream tff_file(file_name, std::ios::in);
        float value;

        // read lines from file and split on whitespace
        if (tff_file.is_open())
        {
            while (tff_file >> value)
            {
                values.push_back(value);
            }
            tff_file.close();
        }
        else
        {
            throw std::runtime_error("Could not open transfer function file " + file_name);
        }
    }

    // Trunctualte if there are too many values respectively fill with zero values.
    // We multiply by 4 because of the values for RGBA-channels.
    values.resize(num_values * 4, 0.0f);

    // create OpenCL 2d image representation
    cl::ImageFormat format;
    format.image_channel_order = CL_RGBA;
    format.image_channel_data_type = CL_FLOAT;
    try
    {
        _tff_mem = cl::Image1D(env->get_properties().context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               format,
                               num_values,
                               values.data(),
                               NULL);
    }
    catch (cl::Error err)
    {
        throw std::runtime_error( "ERROR: " + std::string(err.what()) + "("
                                  + util::get_cl_error_str(err.err()) + ")");
    }

    env->get_garbage_collector().add_mem_object(&_tff_mem);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::create_cl_mem
 */
void trrojan::opencl::volume_raycast_benchmark::create_vol_mem(const scalar_type data_precision,
                                                               const scalar_type sample_precision,
                                                               const std::vector<char> &raw_data,
                                                               const bool use_buffer,
                                                               environment::pointer env)
{
    this->dispatch(scalar_type_list(),
                   data_precision,
                   sample_precision,
                   raw_data,
                   use_buffer,
                   env);
}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(
        const trrojan::configuration &cfg,
        const std::unordered_set<std::string> changed)
{
    // TODO

}


/*
 * trrojan::opencl::volume_raycast_benchmark::generate_kernel
 */
void trrojan::opencl::volume_raycast_benchmark::build_kernel()
{
    // TODO
}


/**
 * trrojan::opencl::volume_raycast_benchmark::update_kernel_args
 */
void trrojan::opencl::volume_raycast_benchmark::update_kernel_args(
        const trrojan::configuration &cfg,
        const std::unordered_set<std::string> changed)
{
    // TODO
}

