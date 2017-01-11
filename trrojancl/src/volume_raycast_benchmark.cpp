/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"

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

    // volume and view properties
    //
    // volume .dat file name is a required factor
    this->_default_configs.add_factor(factor::empty("volume_file_name"));
    // volume resolution from .dat file
    this->_default_configs.add_factor(factor::empty("volume_res_x"));
    this->_default_configs.add_factor(factor::empty("volume_res_y"));
    this->_default_configs.add_factor(factor::empty("volume_res_z"));
    // data precision from .dat file
    this->_default_configs.add_factor(factor::empty("data_precision"));
    this->_default_configs.add_factor(factor::from_manifestations("viewport_width",  1024));
    this->_default_configs.add_factor(factor::from_manifestations("viewport_height", 1024));
    this->_default_configs.add_factor(factor::from_manifestations("sample_precision", 1));
    this->_default_configs.add_factor(factor::from_manifestations("step_size_factor", 0.5));
    // transfer function file name, use a provided linear transfer function as default
    this->_default_configs.add_factor(factor::from_manifestations("tff_file_name", "linear.tff"));
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_x", 0.0));
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_y", 0.0));
    this->_default_configs.add_factor(factor::from_manifestations("view_rot_z", 0.0));
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_x", 0.0));
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_y", 0.0));
    this->_default_configs.add_factor(factor::from_manifestations("view_pos_z",-2.0));

    // rendering modes
    //
    // use linear interpolation (not nearest neighbor interpolation)
    this->_default_configs.add_factor(factor::from_manifestations("use_lerp", true));
    // use early ray termination
    this->_default_configs.add_factor(factor::from_manifestations("use_ERT", true));
    // make a transfer function lookups
    this->_default_configs.add_factor(factor::from_manifestations("use_tff", true));
    // use direct volume rendering (not inderect aka iso-surface rendering)
    this->_default_configs.add_factor(factor::from_manifestations("use_dvr", true));
    // shuffle ray IDs pseudo randomly
    this->_default_configs.add_factor(factor::from_manifestations("shuffle", false));
    // use a linear buffer as volume data structure (instead of a texture)
    this->_default_configs.add_factor(factor::from_manifestations("use_buffer", false));
    // use a simple illumination technique
    this->_default_configs.add_factor(factor::from_manifestations("use_illumination", false));
    // use an orthographic camera projection (instead of a perspective one)
    this->_default_configs.add_factor(factor::from_manifestations("use_ortho_proj", false));

    // debug and misc testing configurations
    //
    // output a rendered image to file (PNG)
    this->_default_configs.add_factor(factor::from_manifestations("img_output", false));
    // count all samples taken along rays
    this->_default_configs.add_factor(factor::from_manifestations("count_samples", false));
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
 *
 */
trrojan::result_set trrojan::opencl::volume_raycast_benchmark::run(
        const configuration_set& configs)
{
    std::vector<std::string> changed;
    result_set retval;

    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto cs = configs;
    cs.merge(this->_default_configs, false);

    cs.foreach_configuration([&](const trrojan::configuration& cs)
    {
        changed.clear();
        this->check_changed_factors(cs, std::back_inserter(changed));
        for (auto& f : cs)
        {
            std::cout << f << std::endl;
        }
        std::cout << std::endl;


        setup_raycaster(cs);

        compose_kernel(cs);

        build_kernel();


        retval.push_back(this->run_kernel());
        return true;
    });

    return result_set();
}


/*
 * setup volume raycaster
 */
void trrojan::opencl::volume_raycast_benchmark::setup_raycaster(
        const trrojan::configuration &cfg)
{
    // TODO
}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(
        const trrojan::configuration &cfg)
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
 * trrojan::opencl::volume_raycast_benchmark::run_kernel
 */
trrojan::result trrojan::opencl::volume_raycast_benchmark::run_kernel()
{
    // TODO

    return trrojan::result();
}
