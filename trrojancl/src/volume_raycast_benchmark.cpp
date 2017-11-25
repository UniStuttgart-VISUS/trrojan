/// <copyright file="volume_raycast_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>

#include "trrojan/opencl/volume_raycast_benchmark.h"

#include <cassert>
#include <random>
#include <numeric>
#define _USE_MATH_DEFINES
#include <math.h>
#include <array>
#include <valarray>

#include "trrojan/image_helper.h"
#include "trrojan/io.h"
#include "trrojan/process.h"
#include "trrojan/timer.h"
#include "trrojan/log.h"

#include "glm/gtc/type_ptr.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/component_wise.hpp"
#include "glm/gtc/matrix_inverse.hpp"

#define _TRROJANSTREAM_DEFINE_FACTOR(f)                                        \
const std::string trrojan::opencl::volume_raycast_benchmark::factor_##f(#f)

_TRROJANSTREAM_DEFINE_FACTOR(environment);
_TRROJANSTREAM_DEFINE_FACTOR(environment_vendor);
_TRROJANSTREAM_DEFINE_FACTOR(device);
_TRROJANSTREAM_DEFINE_FACTOR(device_type);
_TRROJANSTREAM_DEFINE_FACTOR(device_vendor);

_TRROJANSTREAM_DEFINE_FACTOR(iterations);
_TRROJANSTREAM_DEFINE_FACTOR(volume_file_name);
_TRROJANSTREAM_DEFINE_FACTOR(tff_file_name);
_TRROJANSTREAM_DEFINE_FACTOR(viewport);
_TRROJANSTREAM_DEFINE_FACTOR(step_size_factor);

//_TRROJANSTREAM_DEFINE_FACTOR(roll);
//_TRROJANSTREAM_DEFINE_FACTOR(pitch);
//_TRROJANSTREAM_DEFINE_FACTOR(yaw);
//_TRROJANSTREAM_DEFINE_FACTOR(zoom);
_TRROJANSTREAM_DEFINE_FACTOR(cam_position);
_TRROJANSTREAM_DEFINE_FACTOR(cam_rotation);

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
_TRROJANSTREAM_DEFINE_FACTOR(volume_scaling);

#undef _TRROJANSTREAM_DEFINE_FACTOR

// FIXME: OS dependent paths
#ifdef _WIN32
const std::string trrojan::opencl::volume_raycast_benchmark::kernel_snippet_path =
    "\\..\\..\\trrojancl\\include\\kernel\\volume_raycast_snippets";
const std::string trrojan::opencl::volume_raycast_benchmark::kernel_source_path =
    "\\..\\..\\trrojancl\\include\\kernel\\volume_raycast_base.cl";
const std::string trrojan::opencl::volume_raycast_benchmark::test_volume =
    "\\\\trr161store.visus.uni-stuttgart.de\\SFB-TRR 161\\A02\\data\\volumes\\bonsai.dat";
#else   // UNIX
const std::string trrojan::opencl::volume_raycast_benchmark::kernel_snippet_path =
    "/../trrojancl/include/kernel/volume_raycast_snippets";
const std::string trrojan::opencl::volume_raycast_benchmark::kernel_source_path =
    "/../trrojancl/include/kernel/volume_raycast_base.cl";
const std::string trrojan::opencl::volume_raycast_benchmark::test_volume =
//      "/home/brudervn/netshare/trrstore/A02/data/volumes/bonsai.dat";
    "/media/brudervn/Daten/volTest/vol/bonsai.dat";
//    "//trr161store.visus.uni-stuttgart.de/SFB-TRR 161/A02/data/volumes/bonsai.dat";
#endif

/*
 * trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::volume_raycast_benchmark(void)
    : trrojan::benchmark_base("volume_raycast")
    , _model_scale(glm::vec3(1.f))
{

    // default config
    //
    // TODO: @christoph empty factors (aka required factor)
//    this->_default_configs.add_factor(factor::empty("environment"));
//    this->_default_configs.add_factor(factor::from_manifestations(factor_device,
//                                                                  std::string("device")));

    this->_default_configs.add_factor(factor::from_manifestations(
        factor_environment_vendor, static_cast<int>(VENDOR_ANY)));
    this->_default_configs.add_factor(
        factor::from_manifestations(factor_device_type, static_cast<int>(TYPE_GPU)));
    this->_default_configs.add_factor(
        factor::from_manifestations(factor_device_vendor, static_cast<int>(VENDOR_ANY)));

    // if no number of test iterations is specified, use a magic number
    this->_default_configs.add_factor(factor::from_manifestations(factor_iterations, 5));
    // volume and view properties -> basic config
    //
    // volume .dat file name is a required factor
//    this->_default_configs.add_factor(factor::empty("volume_file_name"));
    this->_default_configs.add_factor(factor::from_manifestations(
                                          factor_volume_file_name,
                                          test_volume));
    // transfer function file name, use a provided linear transfer function file as default
    this->_default_configs.add_factor(factor::from_manifestations(factor_tff_file_name,
                                                                  std::string("default")));
    // Down or up-scaling factor for volume data.
    this->_default_configs.add_factor(factor::from_manifestations(factor_volume_scaling, 1.0));

    // camera setup -> kernel runtime factors
    //
    auto viewport = std::array<unsigned int, 2> { 1024, 1024 };
    add_kernel_run_factor(factor_viewport, viewport);
    add_kernel_run_factor(factor_step_size_factor, 0.5);
//    add_kernel_run_factor(factor_roll, 0.0*CL_M_PI);
//    add_kernel_run_factor(factor_pitch, 0.0*CL_M_PI);
//    add_kernel_run_factor(factor_yaw, 0.25*CL_M_PI);
//    add_kernel_run_factor(factor_zoom, -2.0);

    auto pos = std::array<float, 3> {0, 0, 2};
    add_kernel_run_factor(factor_cam_position, pos);
    auto rotation = std::array<float, 4> {1, 0, 0, 0};
    add_kernel_run_factor(factor_cam_rotation, rotation);

    // rendering modes -> kernel build factors
    //
    // sample precision in bytes, if not specified, use uchar (1 byte)
    add_kernel_build_factor(factor_sample_precision,
                            scalar_type_traits<scalar_type::ushort>::name());
    // use linear interpolation (not nearest neighbor interpolation)
    add_kernel_build_factor(factor_use_lerp, false);
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

    // NOTE: parameters for memory pattern test
    // perform a test for memory patterns
    // add_kernel_build_factor(factor_test_memory_patterns, false);
    // pixel offset
    // add_kernel_run_factor(factor_offset_x, 0);
    // add_kernel_run_factor(factor_offset_y, 0);

    // new environment (OpenCL platform) requires new kernel build
    _kernel_build_factors.push_back(factor_environment);
    _kernel_run_factors.push_back(factor_environment);
    _kernel_build_factors.push_back(factor_device);
    _kernel_run_factors.push_back(factor_device);

    // set up camera
    _camera = trrojan::perspective_camera();
}


/*
 * trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark
 */
trrojan::opencl::volume_raycast_benchmark::~volume_raycast_benchmark(void)
{
}


/*
 * trrojan::opencl::volume_raycast_benchmark::can_run
 */
bool trrojan::opencl::volume_raycast_benchmark::can_run(trrojan::environment env,
        trrojan::device device) const noexcept {
    auto d = std::dynamic_pointer_cast<trrojan::opencl::device>(device);
    return (d != nullptr);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::add_kernel_run_factor
 */
void trrojan::opencl::volume_raycast_benchmark::add_kernel_run_factor(std::string name,
                                                                      variant value)
{
    this->_default_configs.add_factor(factor::from_manifestations(name, value));
    this->_kernel_run_factors.push_back(name);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::add_kernel_build_factor
 */
void trrojan::opencl::volume_raycast_benchmark::add_kernel_build_factor(std::string name,
                                                                        variant value)
{
    this->_default_configs.add_factor(factor::from_manifestations(name, value));
    this->_kernel_build_factors.push_back(name);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::run
 */
size_t trrojan::opencl::volume_raycast_benchmark::run(const configuration_set& configs,
                                                      const on_result_callback& result_callback)
{
    std::unordered_set<std::string> changed;
    size_t retval;

    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto cs = configs;
    cs.merge(this->_default_configs, false);

    cs.foreach_configuration([&](trrojan::configuration& cs) -> bool
    {
        auto e = cs.get<trrojan::environment>(environment_base::factor_name);
        auto d = cs.get<trrojan::device>(device_base::factor_name);

        if (!this->can_run(e, d))
        {
            log::instance().write_line(log_level::information, "A "
                                       "benchmark cannot run with the specified combination of "
                                       "environment and device. Skipping it ...");
            return true;
        }

        changed.clear();
        this->check_changed_factors(cs, std::inserter(changed, changed.begin()));

        // setup raycast if OpenCL platform (aka environment) or device changed
        if (changed.count(factor_environment) || changed.count(factor_device))
        {
            setup_raycaster(cs);
            // check vendor and device type
            auto env = cs.find(factor_environment)->value().as<trrojan::environment>();
            environment::pointer env_ptr = std::dynamic_pointer_cast<environment>(env);
            int env_vendor_factor = cs.find(factor_environment_vendor)->value().as<int>();
            if (((env_ptr->get_properties().vendor | VENDOR_ANY) & env_vendor_factor) == 0)
            {
                std::ostringstream os;
                os << "Skipping platform vendor " << util::_vendor_names.find(
                          static_cast<vendor>(env_ptr->get_properties().vendor))->second;
                log::instance().write(log_level::information, os.str().c_str());
                return false;
            }
            auto dev = cs.find(factor_device)->value().as<trrojan::device>();
            device::pointer dev_ptr = std::dynamic_pointer_cast<device>(dev);
            int dev_vendor_factor = cs.find(factor_device_vendor)->value().as<int>();
            unsigned dev_type_factor = cs.find(factor_device_type)->value().as<unsigned>();
            if (((dev_ptr->get_vendor() | VENDOR_ANY) & dev_vendor_factor) == 0)
            {
                std::ostringstream os;
                os << "Skipping device vendor " << util::_vendor_names.find(
                             static_cast<vendor>(dev_vendor_factor))->second;
                log::instance().write(log_level::information, os.str().c_str());
                return false;
            }
            if (((dev_ptr->get_type() | TYPE_ALL) & dev_type_factor) == 0)
            {
                std::ostringstream os;
                os << "Skipping device type " << util::_type_names.find(
                             static_cast<hardware_type>(dev_ptr->get_type()))->second;
                log::instance().write(log_level::information, os.str().c_str());
                return false;
            }
        }
        std::ostringstream os;
        os << "Changed factors: ";
        for (auto& f : changed)
        {
            // output changed config
            os << f << ", ";
        }
        os << std::endl << "Current config: " << std::endl;
        for (auto& f : cs)
        {
            // output current config
            os << f << ", ";
        }
        log::instance().write(log_level::information, os.str().c_str());

        // change the setup according to changed factors that are relevant
        setup_volume_data(cs, changed);

        // compose the OpenCL kernel according to the changed factors,
        // if at least one relevant factor changed
        if (std::any_of(_kernel_build_factors.begin(), _kernel_build_factors.end(),
                        [&](std::string s){return changed.count(s);}))
        {
            // compose the kernel source based on the current config
            compose_kernel(cs);
            // build the kernel file for the current platform (aka environment)
            auto env = cs.find(factor_environment)->value().as<trrojan::environment>();
            auto dev = cs.find(factor_device)->value().as<trrojan::device>();
            auto data_precision = parse_scalar_type(*_passive_cfg.find(factor_data_precision));
            auto sample_precision = parse_scalar_type(*cs.find(factor_sample_precision));
            bool use_buffer = cs.find(factor_use_buffer)->value().as<bool>();
            _precision_div = static_cast<float>(1.0/255.0);        // uchar -> float

            if (!use_buffer)
            {
                if (sample_precision <= data_precision)
                {
                    _precision_div = 1.0f;
                }
                else if (data_precision == scalar_type::uchar
                         && sample_precision == scalar_type::ushort)    // uchar -> ushort
                {
                    _precision_div = 255.0f; // 2**8 - 1
                }
                else if (data_precision == scalar_type::ushort
                         && sample_precision == scalar_type::float32)   // ushort -> float
                {
                    _precision_div = 1.0f/65535.0f; // 2**16 - 1
                }
            }
            else
            {
                if (data_precision == scalar_type::uchar ||
                        sample_precision == scalar_type::uchar)
                {
                    _precision_div = 1.0f;
                }
            }
            build_kernel(std::dynamic_pointer_cast<environment>(env),
                         std::dynamic_pointer_cast<device>(dev),
                         _precision_div,
                         std::string("-DIMAGE_SUPPORT"));
        }

        // reset volume kernel argument if volume data changed
        if (changed.count(factor_volume_file_name))
        {
            _kernel.setArg(VOLUME, _volume_mem);
            cl_float3 model_scale = {_model_scale.x, _model_scale.y, _model_scale.z};
            _kernel.setArg(MODEL_SCALE, model_scale);
        }

        // update the OpenCL kernel arguments according to the changed factors,
        // if at least one relevant factor changed
        if (std::any_of(_kernel_run_factors.begin(), _kernel_run_factors.end(),
                         [&](std::string s){return changed.count(s);}))
        {
            update_kernel_args(cs, changed);
        }
        // set all initial kernel args for the new kernel
        update_initial_kernel_args(cs);

        // run the OpenCL kernel, i.e. the actual test
        auto r = result_callback(std::move(this->run(cs)));
        ++retval;
        return r;
    });

    return retval;
}


/*
 * trrojan::opencl::volume_raycast_benchmark::run
 */
trrojan::result trrojan::opencl::volume_raycast_benchmark::run(const configuration &cfg)
{
    auto env = cfg.find(factor_environment)->value().as<trrojan::environment>();
    environment::pointer env_ptr = std::dynamic_pointer_cast<environment>(env);
    std::vector<double> times(cfg.find(factor_iterations)->value(), 0.0);
    auto imgSize = cfg.find(factor_viewport)->value().as<std::array<unsigned int, 2>>();
    std::array<unsigned int, 3> img_dim = { {imgSize.at(0), imgSize.at(1), 1u} };
    cl_int evt_status = CL_QUEUED;
    for (int i = 0; i < cfg.find(factor_iterations)->value().as<int>(); ++i)
    {
        cl_int evt_status = CL_QUEUED;
        try // opencl scope
        {
            cl::NDRange global_threads(img_dim.at(0), img_dim.at(1));
            cl::Event ndr_evt;
            env_ptr->get_properties().queue.enqueueNDRangeKernel(_kernel,
                                                                 cl::NullRange,
                                                                 global_threads,
                                                                 cl::NullRange,
                                                                 NULL,
                                                                 &ndr_evt);
            env_ptr->get_properties().queue.flush();    // global sync
            while(evt_status != CL_COMPLETE)
            {
                ndr_evt.getInfo<cl_int>(CL_EVENT_COMMAND_EXECUTION_STATUS, &evt_status);
            }
            cl_ulong start = 0;
            cl_ulong end = 0;
            ndr_evt.getProfilingInfo(CL_PROFILING_COMMAND_START, &start);
            ndr_evt.getProfilingInfo(CL_PROFILING_COMMAND_END, &end);
            times.at(i) = static_cast<double>(end - start)*1e-9;
        }
        catch (cl::Error err)
        {
            log_cl_error(err);
        }
    }
    if (cfg.find(factor_img_output)->value().as<bool>())    // output resulting image
    {
        try
        {
            cl::Event read_evt;
            std::array<size_t, 3> origin = {0, 0, 0};
            std::array<size_t, 3> region;
            region[0] = imgSize.at(0);
            region[1] = imgSize.at(1);
            region[2] = 1;
            env_ptr->get_properties().queue.enqueueReadImage(_output_mem,
                                                             CL_TRUE,
                                                             origin,
                                                             region,
                                                             0,
                                                             0,
                                                             _output_data.data(),
                                                             NULL,
                                                             &read_evt);
            env_ptr->get_properties().queue.flush();    // global sync
            evt_status = CL_QUEUED;
            while(evt_status != CL_COMPLETE)
            {
                read_evt.getInfo<cl_int>(CL_EVENT_COMMAND_EXECUTION_STATUS, &evt_status);
            }
        }
        catch (cl::Error err)
        {
            log_cl_error(err);
        }
    }

    if (cfg.find(factor_img_output)->value().as<bool>())
    {
        // write output as image
        auto dev = cfg.find(factor_device)->value().as<trrojan::device>();
        device::pointer dev_ptr = std::dynamic_pointer_cast<device>(dev);
        auto file = cfg.find(factor_volume_file_name)->value().as<std::string>();
        auto rot = cfg.find(factor_cam_rotation)->value().as<std::array<float, 4>>();
        std::size_t found = file.find_last_of("/\\");
        trrojan::save_image("img/" + dev_ptr->name() + "_" + file.substr(found + 1) + "_"
                            + std::to_string(rot.at(0)+rot.at(1)+rot.at(2)+ rot.at(3)) + ".png",
                            _output_data.data(), imgSize.at(0), imgSize.at(1), 4);
    }

    // TODO: move to own method
    // generate result
    trrojan::configuration result_cfg = cfg;
    for (auto& a : _passive_cfg)
    {
        result_cfg.add(a);
    }
    result_cfg.add_system_factors();
    std::vector<std::string> result_names;
    result_names.push_back("execution_time");
    auto retval = std::make_shared<basic_result>(result_cfg, std::move(result_names));

    // calc median of execution times of all runs
    std::sort(times.begin(), times.end());
    double median = times.at(times.size() / 2);
    retval->add({ median });

    std::ostringstream os;
    os << "Kernel time median: " << median << std::endl;
    log::instance().write(log_level::information, os.str().c_str());

    return retval;
}

/**
 * trrojan::opencl::volume_raycast_benchmark::set_shuffled_ray_ids
 */
void trrojan::opencl::volume_raycast_benchmark::set_shuffled_ray_ids(const environment::pointer env,
                                                                     const std::array<unsigned int, 2> viewport)
{
    int pixel_cnt = viewport.at(0) * viewport.at(1);
    // shuffeled ray id array
    std::vector<int> shuffled_ids(pixel_cnt);
    std::iota(std::begin(shuffled_ids), std::end(shuffled_ids), 0);
    unsigned int seed = 42;
    std::shuffle(shuffled_ids.begin(),
                 shuffled_ids.end(),
                 std::default_random_engine(seed));
    _ray_ids = cl::Buffer(env->get_properties().context,
                          CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                          shuffled_ids.size() * sizeof(cl_int),
                          shuffled_ids.data());
}

/*
 * trrojan::opencl::volume_raycast_benchmark::setup_raycaster
 */
void trrojan::opencl::volume_raycast_benchmark::setup_raycaster(const configuration &cfg)
{
    environment::pointer env = std::dynamic_pointer_cast<environment>(
                                cfg.find(factor_environment)->value().as<trrojan::environment>());

    auto f = cfg.find(factor_device);
    auto dev_o = f->value().as<trrojan::device>();
    opencl::device::pointer dev = std::dynamic_pointer_cast<trrojan::opencl::device>(dev_o);

    // set up buffer objects for the view matrix and shuffled IDs
    try
    {
        auto viewport = cfg.find(factor_viewport)->value().as<std::array<unsigned int, 2>>();
        set_shuffled_ray_ids(env, viewport);

        // create OpenCL command queue
        cl_command_queue_properties prop = 0;
        prop |= CL_QUEUE_PROFILING_ENABLE;
        env->create_queue(dev->get(), prop);
    }
    catch (cl::Error err)
    {
        log_cl_error(err);
    }

}

/*
 * trrojan::opencl::volume_raycast_benchmark::setup_volume_data
 */
void trrojan::opencl::volume_raycast_benchmark::setup_volume_data(
        const trrojan::configuration &cfg,
        const std::unordered_set<std::string> changed)
{
    std::vector<char> raw_data;
    // load volume data from dat-raw-file
    if (changed.count(factor_volume_file_name) || changed.count(factor_environment))
    {
        raw_data = load_volume_data(cfg.find(factor_volume_file_name)->value());
    }
    else
    {
        raw_data = _dr.data();
    }

    auto env = cfg.find(factor_environment)->value().as<trrojan::environment>();

    // create OpenCL volume data memory object (either texture or linear buffer)
    if (changed.count(factor_volume_file_name) || changed.count(factor_sample_precision) ||
            changed.count(factor_volume_scaling) || changed.count(factor_environment))
    {
        auto data_precision = parse_scalar_type(*_passive_cfg.find(factor_data_precision));
        auto sample_precision = parse_scalar_type(*cfg.find(factor_sample_precision));

        create_vol_mem(data_precision,
                       sample_precision,
                       raw_data,
                       cfg.find(factor_use_buffer)->value(),
                       std::dynamic_pointer_cast<environment>(env),
                       cfg.find(factor_volume_scaling)->value());
    }
    // transfer function factor changed
    if (changed.count(factor_tff_file_name) || changed.count(factor_environment))
    {
        auto file_name = cfg.find(factor_tff_file_name)->value().as<std::string>();
        load_transfer_function(file_name, std::dynamic_pointer_cast<environment>(env));
    }
}

/**
 * VolumeRenderCL::calcScaling
 */
void trrojan::opencl::volume_raycast_benchmark::calcScaling()
{
    if (!_dr.has_data())
        return;

    glm::vec3 thickness;
    for (size_t i = 0; i < _dr.properties().volume_res.size(); ++i)
    {
        _model_scale[i] = (float)_dr.properties().volume_res.at(i);
        thickness[i] = (float)_dr.properties().slice_thickness.at(i);
    }
    _model_scale *= thickness*(1.f/thickness[0]);
    _model_scale = glm::compMax(_model_scale) / _model_scale;
}

/*
 * trrojan::opencl::volume_raycast_benchmark::load_volume_data
 */
const std::vector<char> & trrojan::opencl::volume_raycast_benchmark::load_volume_data(
        const std::string dat_file)
{
    std::ostringstream os;
    os << "Loading volume data defined in " << dat_file;
    log::instance().write(log_level::information, os.str().c_str());

    try
    {
        _dr.read_files(dat_file);
    }
    catch (std::runtime_error e)
    {
        log::instance().write(log_level::error, e);
    }

    os = std::ostringstream();
    os << _dr.data().size() << " bytes have been read: " << _dr.properties().to_string();
    log::instance().write_line(log_level::information, os.str().c_str());
    calcScaling();

    // update static config
    _passive_cfg.clear();
    if (_dr.properties().format == "UCHAR")
    {
        _passive_cfg.add(named_variant(factor_data_precision,
                                     scalar_type_traits<scalar_type::uchar>::name()));
    }
    else if (_dr.properties().format == "USHORT")
    {
        _passive_cfg.add(named_variant(factor_data_precision,
                                     scalar_type_traits<scalar_type::ushort>::name()));
    }
    else
    {
        log::instance().write_line(log_level::error,
                                   std::invalid_argument(
                                       "Unsupported volume data format defined in dat file."));
    }

    _passive_cfg.add(named_variant(factor_volume_res_x, _dr.properties().volume_res[0]));
    _passive_cfg.add(named_variant(factor_volume_res_y, _dr.properties().volume_res[1]));
    _passive_cfg.add(named_variant(factor_volume_res_z, _dr.properties().volume_res[2]));

    return _dr.data();
}

/**
 * trrojan::opencl::volume_raycast_benchmark::load_transfer_function
 */
void trrojan::opencl::volume_raycast_benchmark::load_transfer_function(
        const std::string file_name,
        environment::pointer env)
{
    std::vector<unsigned char> values;

    if (file_name == "default")
    {
        log::instance().write_line(log_level::warning,
                                   "No transfer function file defined, falling back"
                                   " to default: linear function in range [0;1].");
        // The size of the transfer function vector (256 * RGBA values).
        for (size_t i = 0; i < 256; ++i)
        {
            values.push_back(i);
            values.push_back(0);
            values.push_back(0);
            values.push_back(i);
        }
    }
    else    // try to read file
    {
        std::ostringstream os;
        os << "Loading transfer funtion data defined in " << file_name << std::endl;
        log::instance().write(log_level::information, os.str().c_str());

        std::ifstream tff_file(file_name, std::ios::in);
        float value;

        // read lines from file and split on whitespace
        if (tff_file.is_open())
        {
            while (tff_file >> value)
            {
                values.push_back((char)value);
            }
            tff_file.close();
        }
        else
        {
            log::instance().write_line(log_level::error,
                                       std::runtime_error(
                                           "Could not open transfer function file " + file_name));
        }
    }

    // create OpenCL 1D image representation
    cl::ImageFormat format;
    format.image_channel_order = CL_RGBA;
    // Seems like Intel IGP does not support float textures as input: use uint8 here.
    format.image_channel_data_type = CL_UNORM_INT8;
    try
    {
        _tff_mem = cl::Image1D(env->get_properties().context,
                               CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                               format,
                               values.size() / 4,
                               values.data());
    }
    catch (cl::Error err)
    {
        log_cl_error(err);
    }
}


/*
 * trrojan::opencl::volume_raycast_benchmark::create_cl_mem
 */
void trrojan::opencl::volume_raycast_benchmark::create_vol_mem(const scalar_type data_precision,
                                                               const scalar_type sample_precision,
                                                               const std::vector<char> &raw_data,
                                                               const bool use_buffer,
                                                               environment::pointer env,
                                                               const double scaling_factor)
{
    this->dispatch(scalar_type_list(),
                   data_precision,
                   sample_precision,
                   raw_data,
                   use_buffer,
                   env,
                   scaling_factor);
}


/*
 * Compose the raycastig kernel source
 */
void trrojan::opencl::volume_raycast_benchmark::compose_kernel(
        const trrojan::configuration &cfg)
{
    std::string path = trrojan::get_path(trrojan::get_module_file_name());
    // read all kernel snippets if necessary
    if (_kernel_snippets.empty())
    {
        try
        {
            read_kernel_snippets(path + kernel_snippet_path);
        }
        catch(std::system_error err)
        {
            std::cerr << "ERROR while reading from " << path << kernel_snippet_path
                      << " :\n\t" << err.what() << std::endl;
        }
    }

    // read base kernel file
    _kernel_source = read_text_file(path + kernel_source_path);
    // compose kernel source according to the current config
    //
    if (cfg.find(factor_use_buffer)->value())
    {
        if (parse_scalar_type(*cfg.find(factor_sample_precision)) == scalar_type::uchar)
            replace_keyword("PRECISION", "__global const uchar*", _kernel_source);
        else if (parse_scalar_type(*cfg.find(factor_sample_precision)) == scalar_type::ushort)
            replace_keyword("PRECISION", "__global const ushort*", _kernel_source);
        else if (parse_scalar_type(*cfg.find(factor_sample_precision)) == scalar_type::float32)
            replace_keyword("PRECISION", "__global const float*", _kernel_source);
        else if (parse_scalar_type(*cfg.find(factor_sample_precision)) == scalar_type::float64)
            replace_keyword("PRECISION", "__global const double*", _kernel_source);
    }
    else
    {
        replace_keyword("PRECISION", "__read_only image3d_t", _kernel_source);
    }

    // TODO implement offset for memory pattern test

    if (cfg.find(factor_shuffle)->value())
        replace_kernel_snippet("SHUFFLE", _kernel_source);
    if (cfg.find(factor_use_ortho_proj)->value())
    {
        // TODO: orthogonal camera scaling as zoom
        replace_keyword("CAMERA", _kernel_snippets["ORTHO_CAM"], _kernel_source);
    }
    else
        replace_keyword("CAMERA", _kernel_snippets["PERSPECTIVE_CAM"], _kernel_source);
    if (cfg.find(factor_use_buffer)->value())   // use buffer
    {
        replace_keyword("DATA_SOURCE", _kernel_snippets["BUFFER"], _kernel_source);
        if (cfg.find(factor_use_illumination)->value())
            replace_keyword("ILLUMINATION", _kernel_snippets["ILLUMINATION_BUF"], _kernel_source);
    }
    else    // use texture
    {
        replace_keyword("DATA_SOURCE", _kernel_snippets["TEXTURE"], _kernel_source);
        if (!(cfg.find(factor_use_dvr)->value().as<bool>())) // iso surface rendering
        {
            replace_kernel_snippet("ISO_SURFACE_TEX", _kernel_source);
            if (cfg.find(factor_use_illumination)->value())
                replace_keyword("ILLUMINATION_TEX_ISO", _kernel_snippets["ILLUMINATION_TEX"], _kernel_source);
        }
        if (cfg.find(factor_use_illumination)->value()) // DVR, texture, illumination
            replace_keyword("ILLUMINATION", _kernel_snippets["ILLUMINATION_TEX"], _kernel_source);
    }
    if (cfg.find(factor_use_tff)->value())
        replace_kernel_snippet("TFF_LOOKUP", _kernel_source);
    if (cfg.find(factor_use_ERT)->value())
        replace_kernel_snippet("ERT", _kernel_source);
    if (cfg.find(factor_count_samples)->value())
        replace_kernel_snippet("SAMPLECNT", _kernel_source);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::replace_keyword
 */
void trrojan::opencl::volume_raycast_benchmark::replace_keyword(const std::string keyword,
                                                                const std::string insert,
                                                                std::string &text,
                                                                const std::string prefix,
                                                                const std::string suffix)
{
    std::string kernel_keyword = std::string(prefix + keyword + suffix);
    std::size_t pos = text.find(kernel_keyword);
    std::size_t len = kernel_keyword.length();
    if (pos != std::string::npos)
    {
        text.replace(pos, len, insert);
    }
    else
    {
        throw std::invalid_argument("Could not find keyword " + keyword);
    }
}


/*
 * trrojan::opencl::volume_raycast_benchmark::replace_kernel_snippet
 */
void trrojan::opencl::volume_raycast_benchmark::replace_kernel_snippet(const std::string keyword,
                                                                       std::string &kernel_source)
{
    if (_kernel_snippets.empty())
    {
        throw std::runtime_error("No kernel snippets where loaded yet.");
    }
    replace_keyword(keyword, _kernel_snippets[keyword], kernel_source);
}


/*
 * trrojan::opencl::volume_raycast_benchmark::generate_kernel
 */
void trrojan::opencl::volume_raycast_benchmark::build_kernel(environment::pointer env,
                                                             device::pointer dev,
                                                             const float precision_div,
                                                             const std::string build_flags)
{
//    std::cout << _kernel_source << std::endl; // DEBUG: print out composed kernel source
    cl::Program::Sources source; // (1, std::make_pair(_kernel_source.data(), _kernel_source.size()));
    source.push_back(_kernel_source);
    try
    {
        env->generate_program(source);
        const std::vector<cl::Device> device = {dev.get()->get()};
        env->get_properties().program.build(device, build_flags.c_str());
        std::string str = env->get_properties().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(
                    dev.get()->get());
        log::instance().write(log_level::information, "OpenCL kernel successfully built.");
        if (!(str.length() > 0))
            log::instance().write(log_level::information, str.c_str());

        _kernel = cl::Kernel(env->get_properties().program, "volumeRender", NULL);
        // set default kernel arguments and buffer
        set_kernel_args(precision_div);
    }
    catch (cl::Error err)
    {
        if (err.err() == CL_BUILD_PROGRAM_FAILURE)
        {
            std::ostringstream os;
            os << "Error building volume raycasting kernel." << std::endl;
            // print out compiler output on build error
            std::string str = env->get_properties().program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(
                        dev.get()->get());
            os << " ***************** BUILD LOG *******************\n";
            os << str << std::endl;
            os << " ***********************************************\n";

            log::instance().write(log_level::error, os.str().c_str());
        }
        else
            log_cl_error(err);
    }
}


/**
 * trrojan::opencl::volume_raycast_benchmark::set_kernel_args
 */
void trrojan::opencl::volume_raycast_benchmark::set_kernel_args(const float precision_div)
{
    try
    {
        _kernel.setArg(VOLUME, _volume_mem);
        _kernel.setArg(TFF, _tff_mem);
        cl_float16 view_mat = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  0, 0, 0, 1};
        _kernel.setArg(VIEW, view_mat);
        _kernel.setArg(ID, _ray_ids);
        _kernel.setArg(PRECISION, precision_div);
        cl_float3 model_scale = {_model_scale.x, _model_scale.y, _model_scale.z};
        _kernel.setArg(MODEL_SCALE, model_scale);
    }
    catch (cl::Error err)
    {
        log_cl_error(err);
    }
}

/**
 * trrojan::opencl::volume_raycast_benchmark::update_all_kernel_args
 */
void trrojan::opencl::volume_raycast_benchmark::update_initial_kernel_args(
        const trrojan::configuration &cfg)
{
    try
    {
        _kernel.setArg(VOLUME, _volume_mem);
        _kernel.setArg(OUTPUT, _output_mem);
        _kernel.setArg(TFF, _tff_mem);
        _kernel.setArg(ID, _ray_ids);
        _kernel.setArg(STEP_SIZE, static_cast<cl_float>(cfg.find(factor_step_size_factor)->value()));
        cl_uint3 resolution = {{_volume_res[0], _volume_res[1], _volume_res[2]}};
        _kernel.setArg(RESOLUTION, resolution);
        _kernel.setArg(SAMPLER, _sampler);
        _kernel.setArg(PRECISION, _precision_div);
        cl_float3 model_scale = {_model_scale.x, _model_scale.y, _model_scale.z};
        _kernel.setArg(MODEL_SCALE, model_scale);

        // TODO move to own method
        auto pos = cfg.find(factor_cam_position)->value().as<std::array<float, 3>>();
        _camera.set_look_from(glm::vec3(pos.at(0), pos.at(1), pos.at(2)));
        auto rot = cfg.find(factor_cam_rotation)->value().as<std::array<float, 4>>();
        _camera.rotate_fixed_to(glm::quat(rot.at(0), rot.at(1), rot.at(2), rot.at(3)));
        glm::mat4 view = _camera.get_inverse_view_mx();
        cl_float16 view_mat = {view[0][0], view[1][0], view[2][0], view[3][0],
                               view[0][1], view[1][1], view[2][1], view[3][1],
                               view[0][2], view[1][2], view[2][2], view[3][2],
                               view[0][3], view[1][3], view[2][3], view[3][3]};
        _kernel.setArg(VIEW, view_mat);
    }
    catch (cl::Error err)
    {
        log_cl_error(err);
    }
}



/**
 * trrojan::opencl::volume_raycast_benchmark::update_kernel_args
 */
void trrojan::opencl::volume_raycast_benchmark::update_kernel_args(
        const trrojan::configuration &cfg,
        const std::unordered_set<std::string> changed)
{
    auto env = cfg.find(factor_environment)->value().as<trrojan::environment>();
    environment::pointer env_ptr = std::dynamic_pointer_cast<environment>(env);

    if (changed.count(factor_cam_position) + changed.count(factor_cam_rotation)
            + changed.count(factor_device))
    {
        auto pos = cfg.find(factor_cam_position)->value().as<std::array<float, 3>>();
        _camera.set_look_from(glm::vec3(pos.at(0), pos.at(1), pos.at(2)));
        auto rot = cfg.find(factor_cam_rotation)->value().as<std::array<float, 4>>();
        _camera.rotate_fixed_to(glm::quat(rot.at(0), rot.at(1), rot.at(2), rot.at(3)));
        glm::mat4 view = _camera.get_inverse_view_mx();

        cl_float16 view_mat = {view[0][0], view[1][0], view[2][0], view[3][0],
                               view[0][1], view[1][1], view[2][1], view[3][1],
                               view[0][2], view[1][2], view[2][2], view[3][2],
                               view[0][3], view[1][3], view[2][3], view[3][3]};
        try
        {
            _kernel.setArg(VIEW, view_mat);
        }
        catch (cl::Error err)
        {
            log_cl_error(err);
        }
    }
    // interpolation
    if (changed.count(factor_use_lerp) || changed.count(factor_device))
    {
        try
        {
            if (cfg.find(factor_use_lerp)->value().as<bool>()) // linear
            {
                _sampler = cl::Sampler(env_ptr->get_properties().context,
                                      CL_TRUE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_LINEAR);
            }
            else // nearest
            {
                _sampler = cl::Sampler(env_ptr->get_properties().context,
                                      CL_TRUE, CL_ADDRESS_CLAMP_TO_EDGE, CL_FILTER_NEAREST);
            }
            _kernel.setArg(SAMPLER, _sampler);
        }
        catch (cl::Error err)
        {
            log_cl_error(err);
        }
    }
    // viewport or device (generate output image)
    if (changed.count(factor_viewport) || changed.count(factor_device))
    {
        cl::ImageFormat format;
        format.image_channel_order = CL_RGBA;
        format.image_channel_data_type = CL_FLOAT;
        try
        {
            auto imgSize = cfg.find(factor_viewport)->value().as<std::array<unsigned int, 2>>();
            _output_mem = cl::Image2D(env_ptr->get_properties().context,
                                      CL_MEM_WRITE_ONLY,
                                      format,
                                      imgSize.at(0),
                                      imgSize.at(1));
            _kernel.setArg(OUTPUT, _output_mem);
            set_shuffled_ray_ids(env_ptr, imgSize);

            _output_data.resize(imgSize.at(0) * imgSize.at(1) * 4, 0);
        }
        catch (cl::Error err)
        {
            log_cl_error(err);
        }
    }
    if (changed.count(factor_step_size_factor) || changed.count(factor_device))
    {
        try{
            _kernel.setArg(STEP_SIZE,
                           static_cast<cl_float>(cfg.find(factor_step_size_factor)->value()));
        } catch (cl::Error err) {
            log_cl_error(err);
        }
    }
    if (changed.count(factor_volume_file_name) || changed.count(factor_device)
            || changed.count(factor_volume_scaling))
    {
        // TODO from static config -> merge
        cl_uint3 resolution = {{_volume_res[0], _volume_res[1], _volume_res[2]}};
        try{
            _kernel.setArg(RESOLUTION, resolution);
        } catch (cl::Error err) {
            log_cl_error(err);
        }
    }
    // TODO: include optional ray id offsets
//    if (changed.count(factor_offset_x) || changed.count(factor_offset_y))
//    {
//        cl_int2 offset = {{cfg.find(factor_offset_x)->value(), cfg.find(factor_offset_y)->value()}};
//        try{
//            _kernel.setArg(OFFSET, offset);
//        } catch (cl::Error err) {
//            log_cl_error(err);
//        }
//    }
}


/**
 * trrojan::opencl::volume_raycast_benchmark::log_cl_error
 */
void trrojan::opencl::volume_raycast_benchmark::log_cl_error(cl::Error error)
{
    log::instance().write(log_level::error, std::runtime_error(
                              std::string(error.what()) + "(" +
                              util::get_cl_error_str(error.err()) + ")"));
}


/*
 * trrojan::opencl::volume_raycast_benchmark::create_view_mat
 */
cl_float16 trrojan::opencl::volume_raycast_benchmark::create_view_mat(double roll,
                                                                      double pitch,
                                                                      double yaw,
                                                                      double zoom)
{
    // Convert Euler Angles(roll, pitch, yaw) to axes(x, y, z).
    // We use a right handed coordinate system.
    // Assuming radians (not degrees)!
    if (roll > CL_M_PI*2.0 || pitch > CL_M_PI*2.0 || yaw > CL_M_PI*2.0)
    {
        std::cerr << "WARNING: One or more rotation parameters are greater than 2*pi. "
                     "Rotation parameters are always interpreted as radians!" << std::endl;
    }

    std::valarray<double> s(_dr.properties().volume_res.size());
    for (size_t i = 0; i < _dr.properties().volume_res.size(); ++i)
    {
        s[i] = _dr.properties().volume_res.at(i);
    }
    std::valarray<double> thickness(_dr.properties().slice_thickness.data(),
                                    _dr.properties().slice_thickness.size());
    s *= thickness*(1.0/thickness[0]);
#undef max  // error here if I don't undef max
    s = s.max() / s;
    std::ostringstream os;
    os << "Scaling volume: (" << s[0] << ", " << s[1] << ", " << s[2] << ")";
    log::instance().write(log_level::information, os.str().c_str());

    float zoom_f = static_cast<float>(zoom);
    // The order of rotation is Roll -> Yaw -> Pitch (Rx*Ry*Rz)
    float cx = static_cast<float>(cos(pitch));
    float sx = static_cast<float>(sin(pitch));
    float cy = static_cast<float>(cos(yaw));
    float sy = static_cast<float>(sin(yaw));
    float cz = static_cast<float>(cos(roll));
    float sz = static_cast<float>(sin(roll));

    // The resulting matrix is already transposed (!)
    std::array<float, 3> x_axis = {cy*cz, sx*sy*cz + cx*sz, -cx*sy*cz + sx*sz};
    std::array<float, 3> y_axis = {-cy*sz, -sx*sy*sz + cx*cz, cx*sy*sz + sx*cz};
    std::array<float, 3> z_axis = {sy, -sx*cy, cx*cy};

    return cl_float16
    {{
        (float)s[0]*x_axis[0], (float)s[0]*x_axis[1], (float)s[0]*x_axis[2], -x_axis[2]*zoom_f*(float)s[0],
        (float)s[1]*y_axis[0], (float)s[1]*y_axis[1], (float)s[1]*y_axis[2], -y_axis[2]*zoom_f*(float)s[1],
        (float)s[2]*z_axis[0], (float)s[2]*z_axis[1], (float)s[2]*z_axis[2], -z_axis[2]*zoom_f*(float)s[2],
                0,              0,              0,                  1
    }};
}

/*
 * trrojan::opencl::volume_raycast_benchmark::read_kernel_snippets
 */
void trrojan::opencl::volume_raycast_benchmark::read_kernel_snippets(const std::string path)
{
    std::vector<std::string> paths;
    get_file_system_entries(std::back_inserter(paths), path, false,
                            trrojan::has_extension(".cl"));
    // clear old map entries if any
    _kernel_snippets.clear();
    // read new ones
    for (const auto& path : paths)
    {
        _kernel_snippets[get_file_name(path, false)] = read_text_file(path);
    }
}
