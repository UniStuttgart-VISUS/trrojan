/// <copyright file="mmpld_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/d3d11/mmpld_benchmark.h"

#include <cinttypes>

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

//#include "SphereVertexShader.h"


/*
 * trrojan::d3d11::mmpld_benchmark::to_d3d11
 */
std::vector<D3D11_INPUT_ELEMENT_DESC> trrojan::d3d11::mmpld_benchmark::to_d3d11(
        const mmpld_reader::list_header& header) {
    D3D11_INPUT_ELEMENT_DESC element;
    UINT offset = 0;
    std::vector<D3D11_INPUT_ELEMENT_DESC> retval;

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "POSITION";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (header.vertex_type) {
        case mmpld_reader::vertex_type::float_xyz:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::vertex_type::float_xyzr:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case  mmpld_reader::vertex_type::short_xyz:
            throw std::runtime_error("vertex_type::short_xyz is not "
                "supported with Direct3D 11.");
    }

    ::memset(&element, 0, sizeof(element));
    element.SemanticName = "COLOR";
    element.AlignedByteOffset = offset;
    element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

    switch (header.colour_type) {
        case mmpld_reader::colour_type::float_i:
            element.Format = DXGI_FORMAT_R32_FLOAT;
            offset += 1 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::float_rgb:
            element.Format = DXGI_FORMAT_R32G32B32_FLOAT;
            offset += 3 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::float_rgba:
            element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;

        case mmpld_reader::colour_type::uint8_rgb:
            throw std::runtime_error("colour_type::uint8_rgb is not "
                "supported with Direct3D 11.");

        case mmpld_reader::colour_type::uint8_rgba:
            element.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            offset += 4 * sizeof(float);
            retval.push_back(element);
            break;
    }

    return std::move(retval);
}


/*
 * trrojan::d3d11::mmpld_benchmark::mmpld_benchmark
 */
trrojan::d3d11::mmpld_benchmark::mmpld_benchmark(void)
        : trrojan::benchmark_base("mmpld-raycaster") {
    //// If no scalar type is specfieid, use 64-bit float.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_scalar_type, scalar_type_traits<scalar_type::float64>::name()));

    //// If no scalar is specified, use a magic number.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_scalar, 42));

    //// If no access pattern is specified, test all.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_access_pattern, { ap_traits<access_pattern::contiguous>::name(),
    //    ap_traits<access_pattern::interleaved>::name() }));

    //// If no number of iterations is specified, use a magic number.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_iterations, 10));

    //// If no number of threads is specifed, use all possible values up
    //// to the number of logical processors in the system.
    //auto lc = system_factors::instance().logical_cores().as<uint32_t>();
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_threads, { 1u, lc }));

    //// If no problem size is given, test all all of them.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_problem_size,
    //    //8000000));
    //    worker_thread::problem_sizes::to_vector()));

    //// Enable all tasks by default.
    //this->_default_configs.add_factor(factor::from_manifestations(
    //    factor_task_type, { task_type_traits<task_type::add>::name(),
    //    task_type_traits<task_type::copy>::name(),
    //    task_type_traits<task_type::scale>::name(),
    //    task_type_traits<task_type::triad>::name() }));
}


/*
 * trrojan::d3d11::mmpld_benchmark::~mmpld_benchmark
 */
trrojan::d3d11::mmpld_benchmark::~mmpld_benchmark(void) { }


/*
 * trrojan::d3d11::mmpld_benchmark::run
 */
size_t trrojan::d3d11::mmpld_benchmark::run(const configuration_set& configs,
        const on_result_callback& callback) {
    std::vector<std::string> changed;
    size_t retval = 0;

#if 0
    // Check that caller has provided all required factors.
    this->check_required_factors(configs);

    // Merge missing factors from default configuration.
    auto c = configs;
    c.merge(this->_default_configs, false);

    // Invoke each configuration.
    c.foreach_configuration([&](const trrojan::configuration& c) {
        //changed.clear();
        //this->check_changed_factors(c, std::back_inserter(changed));
        //this->log_run(c);
        //std::cout << std::endl;
        //retval.push_back(this->run(c));
        //return true;
        // TODO: optimise reallocs.
        try {
            this->log_run(c);
            ++retval;
            return callback(std::move(this->run(c)));
        } catch (const std::exception& ex) {
            log::instance().write_line(ex);
            return false;
        }
    });
#endif

    return retval;
}


/*
 * trrojan::d3d11::mmpld_benchmark::run
 */
trrojan::result trrojan::d3d11::mmpld_benchmark::run(
        const configuration& config) {
    //auto problem = stream_benchmark::to_problem(config);
    //auto threads = worker_thread::create(problem);
    //worker_thread::join(threads.begin(), threads.end());
    //return stream_benchmark::collect_results(config, problem, threads.begin(),
    //    threads.end());
    throw 1;
}
