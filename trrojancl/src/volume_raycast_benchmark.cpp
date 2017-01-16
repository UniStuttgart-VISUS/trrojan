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
                                                                  std::string("/media/brudervn/Daten/volTest/vol/bonsai.dat")));

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

    if (changed.count("volume_file_name") || changed.count("sample_precision"))
    {
        static_cfg = load_volume_data(cfg.find("volume_file_name")->value(),
                                      cfg.find("sample_precision")->value());
    }

    if (changed.count("tff_file_name"))
    {
        // TODO
        //load_transfer_function(  );
    }
}


/**
 * trrojan::opencl::volume_raycast_benchmark::load_volume_data
 */
trrojan::configuration trrojan::opencl::volume_raycast_benchmark::load_volume_data(
        const std::string dat_file,
        const unsigned int sample_precision)
{
    std::cout << "Loading volume data defined in " << dat_file << std::endl;
    trrojan::configuration static_cfg;

    trrojan::timer t;
//t.start();
    dat_raw_reader dr;
    try
    {
        dr.read_files(dat_file);
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl;
        return static_cfg;
    }

    const std::vector<char> &raw_data = dr.data();
    std::cout << raw_data.size() << " bytes have been read." << std::endl;
    std::cout << dr.properties().to_string() << std::endl;

    static_cfg.add(named_variant("data_precision", dr.properties().format));
    static_cfg.add(named_variant("volume_res_x", dr.properties().volume_res[0]));
    static_cfg.add(named_variant("volume_res_y", dr.properties().volume_res[1]));
    static_cfg.add(named_variant("volume_res_z", dr.properties().volume_res[2]));

//std::cout << t.elapsed_millis() << std::endl;
//t.start();
    if (dr.properties().format == "USHORT" || sample_precision > 1u)
    {
        size_t voxel_cnt = static_cast<size_t>(dr.properties().volume_res[0]) *
                           static_cast<size_t>(dr.properties().volume_res[1]) *
                           static_cast<size_t>(dr.properties().volume_res[2]);

        // TODO: find a better solution than this 8 cases cascade...
        if (dr.properties().format == "UCHAR")
        {
            switch (sample_precision)
            {
            case 1:
                // TODO direct cast?
                convert_data_precision<unsigned char, unsigned char>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 2:
                convert_data_precision<unsigned char, unsigned short>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 4:
                convert_data_precision<unsigned char, float>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 8:
                convert_data_precision<unsigned char, double>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            default:
                throw std::invalid_argument(
                            "Precision must be 1, 2, 4 or 8 bytes but is "
                            + std::to_string(sample_precision) + " bytes");
                break;
            }

        }
        else if (dr.properties().format == "USHORT")
        {
            switch (sample_precision)
            {
            case 1:
                convert_data_precision<unsigned short, unsigned char>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 2:
                // TODO direct cast?
                convert_data_precision<unsigned short, unsigned short>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 4:
                convert_data_precision<unsigned short, float>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            case 8:
                convert_data_precision<unsigned short, double>(
                            raw_data, voxel_cnt, sample_precision);
                break;
            default:
                throw std::invalid_argument(
                            "Precision must be 1, 2, 4 or 8 bytes but is "
                            + std::to_string(sample_precision) + " bytes");
                break;
            }
        }
        else
        {
            throw std::invalid_argument("Volume data format must be UCHAR or USHORT.");
        }
//std::cout << t.elapsed_millis() << std::endl;
    }

    return static_cfg;
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
void trrojan::opencl::volume_raycast_benchmark::update_kernel_args(const trrojan::configuration &cfg,
                                                                   const std::tr1::unordered_set<std::string> changed)
{
    // TODO
}

