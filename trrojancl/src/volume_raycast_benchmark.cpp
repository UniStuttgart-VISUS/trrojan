/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"

#include "trrojan/timer.h"


#define _TRROJANSTREAM_DEFINE_FACTOR(f)                                        \
const std::string trrojan::opencl::volume_raycast_benchmark::factor_##f(#f)

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
    // if no number of test iterations is specified, use a magic number
    this->_default_configs.add_factor(factor::from_manifestations(factor_iterations, 5));

    // volume and view properties -> basic config
    //
    // volume .dat file name is a required factor
//    this->_default_configs.add_factor(factor::empty("volume_file_name"));
    this->_default_configs.add_factor(factor::from_manifestations(factor_volume_file_name,
                                                                  std::string("/media/brudervn/Daten/volTest/vol/chameleon.dat")));

    // transfer function file name, use a provided linear transfer function file as default
    this->_default_configs.add_factor(factor::from_manifestations(factor_tff_file_name,
                                                                  std::string("linear.tff")));

    // camera setup -> kernel runtime factors
    //
    this->_default_configs.add_factor(factor::from_manifestations(factor_viewport_width,  1024));
    _kernel_run_factors.push_back(factor_viewport_width);
    this->_default_configs.add_factor(factor::from_manifestations(factor_viewport_height, 1024));
    _kernel_run_factors.push_back(factor_viewport_height);
    this->_default_configs.add_factor(factor::from_manifestations(factor_step_size_factor, 0.5));
    _kernel_run_factors.push_back(factor_step_size_factor);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_rot_x, 0.0));
    _kernel_run_factors.push_back(factor_view_rot_x);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_rot_y, 0.0));
    _kernel_run_factors.push_back(factor_view_rot_y);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_rot_z, 0.0));
    _kernel_run_factors.push_back(factor_view_rot_z);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_pos_x, 0.0));
    _kernel_run_factors.push_back(factor_view_pos_x);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_pos_y, 0.0));
    _kernel_run_factors.push_back(factor_view_pos_y);
    this->_default_configs.add_factor(factor::from_manifestations(factor_view_pos_z,-2.0));
    _kernel_run_factors.push_back(factor_view_pos_z);

    // rendering modes -> kernel build factors
    //
    // sample precision in bytes, if not specified, use uchar (1 byte)
    this->_default_configs.add_factor(factor::from_manifestations(
                                          factor_sample_precision,
                                          scalar_type_traits<scalar_type::uchar>::name()));
    _kernel_build_factors.push_back(factor_sample_precision);
    // use linear interpolation (not nearest neighbor interpolation)
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_lerp, true));
    _kernel_build_factors.push_back(factor_use_lerp);
    // use early ray termination
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_ERT, true));
    _kernel_build_factors.push_back(factor_use_ERT);
    // make a transfer function lookups
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_tff, true));
    _kernel_build_factors.push_back(factor_use_tff);
    // use direct volume rendering (not inderect aka iso-surface rendering)
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_dvr, true));
    _kernel_build_factors.push_back(factor_use_dvr);
    // shuffle ray IDs pseudo randomly
    this->_default_configs.add_factor(factor::from_manifestations(factor_shuffle, false));
    _kernel_build_factors.push_back(factor_shuffle);
    // use a linear buffer as volume data structure (instead of a texture)
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_buffer, false));
    _kernel_build_factors.push_back(factor_use_buffer);
    // use a simple illumination technique
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_illumination, false));
    _kernel_build_factors.push_back(factor_use_illumination);
    // use an orthographic camera projection (instead of a perspective one)
    this->_default_configs.add_factor(factor::from_manifestations(factor_use_ortho_proj, false));
    _kernel_build_factors.push_back(factor_use_ortho_proj);

    // debug and misc testing configurations
    //
    // output a rendered image to file (PNG) -> basic config
    this->_default_configs.add_factor(factor::from_manifestations(factor_img_output, false));
    // count all samples taken along rays -> kernel build factor
    this->_default_configs.add_factor(factor::from_manifestations(factor_count_samples, false));
    _kernel_build_factors.push_back(factor_count_samples);
    // perform a test for memory patterns
    // this->_default_configs.add_factor(factor::from_manifestations("test_memory_patterns", false));
    // pixel offset
    // this->_default_configs.add_factor(factor::from_manifestations("offset_x", 0));
    // this->_default_configs.add_factor(factor::from_manifestations("offset_y", 0));
}

/*
 * trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark(void)
{
}


/*
 * trrojan::opencl::volume_raycast_benchmark::run
 */
size_t trrojan::opencl::volume_raycast_benchmark::run(
        const configuration_set& configs, const on_result_callback& callback)
{
    std::tr1::unordered_set<std::string> changed;
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
        const std::tr1::unordered_set<std::string> changed)
{
    // contains the static factors that are defined through the volume data set ".dat" file
    trrojan::configuration static_cfg;
    std::vector<char> raw_data;

    // load volume data from dat-raw-file
    if (changed.count(factor_volume_file_name))
    {
        raw_data = load_volume_data(cfg.find(factor_volume_file_name)->value(), static_cfg);
    }

    // create OpenCL memory object
    if (changed.count(factor_sample_precision))
    {
        auto data_precision = parse_scalar_type(*static_cfg.find(factor_data_precision));
        auto sample_precision = parse_scalar_type(*cfg.find(factor_sample_precision));

        // TODO include the famous enum_pase_helper
        // TODO unify factor strings (.dat strings conversion) for precisions

        create_cl_mem(data_precision,
                      sample_precision,
                      raw_data,
                      cfg.find(factor_use_buffer)->value());
    }

    if (changed.count(factor_tff_file_name))
    {
        // TODO
        //load_transfer_function(  );
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

    trrojan::timer t;
//t.start();
    try
    {
        _dr.read_files(dat_file);
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl;
        throw e;
    }

//    raw_data = _dr.data();
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


/*
 * trrojan::opencl::volume_raycast_benchmark::create_cl_mem
 */
void trrojan::opencl::volume_raycast_benchmark::create_cl_mem(const scalar_type data_precision,
                                                              const scalar_type sample_precision,
                                                              const std::vector<char> &raw_data,
                                                              const bool use_buffer)
{
    this->dispatch(scalar_type_list(), data_precision, sample_precision, raw_data, use_buffer);
}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(
        const trrojan::configuration &cfg,
        const std::tr1::unordered_set<std::string> changed)
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
        const std::tr1::unordered_set<std::string> changed)
{
    // TODO
}

