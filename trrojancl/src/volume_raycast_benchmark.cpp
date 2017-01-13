/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"
#include "trrojan/opencl/dat_raw_reader.h"

#include "trrojan/timer.h"

/*
 * trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark(void)
    : trrojan::benchmark_base("volume_raycast")
{
    // default config
    //
    // if no number of test iterations is specified, use a magic number
    this->_default_configs.add_factor(factor::from_manifestations("iterations", 5));

    // volume and view properties -> basic config
    //
    // volume .dat file name is a required factor
//    this->_default_configs.add_factor(factor::empty("volume_file_name"));
    this->_default_configs.add_factor(factor::from_manifestations("volume_file_name",
                                                                  std::string("/media/brudervn/Daten/volTest/vol/chameleon.dat")));

    // TODO: remove commented section
//    // volume resolution from .dat file // TODO: "auto fill factor"
//    this->_default_configs.add_factor(factor::empty("volume_res_x"));
//    this->_default_configs.add_factor(factor::empty("volume_res_y"));
//    this->_default_configs.add_factor(factor::empty("volume_res_z"));
//    // data precision from .dat file
//    this->_default_configs.add_factor(factor::empty("data_precision"));

    // transfer function file name, use a provided linear transfer function file as default
    this->_default_configs.add_factor(factor::from_manifestations("tff_file_name",
                                                                  std::string("linear.tff")));

    // camera setup -> kernel runtime factors
    //
    this->_default_configs.add_factor(factor::from_manifestations("viewport_width",  1024));
    _kernel_run_factors.push_back("viewport_width");
    this->_default_configs.add_factor(factor::from_manifestations("viewport_height", 1024));
    _kernel_run_factors.push_back("viewport_height");
    this->_default_configs.add_factor(factor::from_manifestations("step_size_factor", 0.5));
    _kernel_run_factors.push_back("step_size_factor");
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_x", 0.0));
    _kernel_run_factors.push_back("view_rot_x");
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_y", 0.0));
    _kernel_run_factors.push_back("view_rot_y");
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_z", 0.0));
    _kernel_run_factors.push_back("view_rot_z");
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_x", 0.0));
    _kernel_run_factors.push_back("view_pos_x");
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_y", 0.0));
    _kernel_run_factors.push_back("view_pos_y");
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_z",-2.0));
    _kernel_run_factors.push_back("view_pos_z");

    // rendering modes -> kernel build factors
    //
    // sample precision in bytes
    this->_default_configs.add_factor(factor::from_manifestations("sample_precision", 1));
    _kernel_build_factors.push_back("sample_precision");
    // use linear interpolation (not nearest neighbor interpolation)
    this->_default_configs.add_factor(factor::from_manifestations("use_lerp", true));
    _kernel_build_factors.push_back("use_lerp");
    // use early ray termination
    this->_default_configs.add_factor(factor::from_manifestations("use_ERT", true));
    _kernel_build_factors.push_back("use_ERT");
    // make a transfer function lookups
    this->_default_configs.add_factor(factor::from_manifestations("use_tff", true));
    _kernel_build_factors.push_back("use_tff");
    // use direct volume rendering (not inderect aka iso-surface rendering)
    this->_default_configs.add_factor(factor::from_manifestations("use_dvr", true));
    _kernel_build_factors.push_back("use_dvr");
    // shuffle ray IDs pseudo randomly
    this->_default_configs.add_factor(factor::from_manifestations("shuffle", false));
    _kernel_build_factors.push_back("shuffle");
    // use a linear buffer as volume data structure (instead of a texture)
    this->_default_configs.add_factor(factor::from_manifestations("use_buffer", false));
    _kernel_build_factors.push_back("use_buffer");
    // use a simple illumination technique
    this->_default_configs.add_factor(factor::from_manifestations("use_illumination", false));
    _kernel_build_factors.push_back("use_illumination");
    // use an orthographic camera projection (instead of a perspective one)
    this->_default_configs.add_factor(factor::from_manifestations("use_ortho_proj", false));
    _kernel_build_factors.push_back("use_ortho_proj");

    // debug and misc testing configurations
    //
    // output a rendered image to file (PNG) -> basic config
    this->_default_configs.add_factor(factor::from_manifestations("img_output", false));
    // count all samples taken along rays -> kernel build factor
    this->_default_configs.add_factor(factor::from_manifestations("count_samples", false));
    _kernel_build_factors.push_back("count_samples");
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
trrojan::result_set trrojan::opencl::volume_raycast_benchmark::run(
        const configuration_set& configs)
{
    std::tr1::unordered_set<std::string> changed;
    result_set retval;

    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto cs = configs;
    cs.merge(this->_default_configs, false);

    cs.foreach_configuration([&](const trrojan::configuration& cs)
    {
        changed.clear();
        this->check_changed_factors(cs, std::inserter(changed, changed.begin()));
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
        retval.push_back(this->run(cs));
        return true;
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
void trrojan::opencl::volume_raycast_benchmark::setup_raycaster(const trrojan::configuration &cfg,
                                                                const std::tr1::unordered_set<std::string> changed)
{
    // TODO
    if (changed.count("volume_file_name") || changed.count("data_precision"))
    {
        auto it = std::find_if(cfg.begin(), cfg.end(),
                           [](const named_variant& el){ return el.name() == "volume_file_name";});
        std::string file_name = it->value();

        it = std::find_if(cfg.begin(), cfg.end(),
                                   [](const named_variant& el){ return el.name() == "data_precision";});
        int data_precision = it->value();

        load_volume_data(file_name, data_precision);
    }
    if (changed.count("tff_file_name"))
    {
        //load_transfer_function(  );
    }
}


/**
 * trrojan::opencl::volume_raycast_benchmark::load_volume_data
 */
void trrojan::opencl::volume_raycast_benchmark::load_volume_data(const std::string dat_file,
                                                                 unsigned int data_precision)
{
    std::cout << "Loading volume data defined in " << dat_file << std::endl;

    trrojan::timer t;
    t.start();
    dat_raw_reader dr;
    try
    {
        dr.read_files(dat_file);
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }

    const std::vector<char> &raw_data = dr.data();
    std::cout << raw_data.size() << " bytes have been read." << std::endl;


    // TODO
}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(const trrojan::configuration &cfg,
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
void trrojan::opencl::volume_raycast_benchmark::update_kernel_args(const trrojan::configuration &cfg,
                                                                   const std::tr1::unordered_set<std::string> changed)
{
    // TODO
}

