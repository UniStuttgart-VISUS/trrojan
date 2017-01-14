/// <copyright file="stream_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/stream_benchmark.h"

#include <cinttypes>

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/system_factors.h"
#include "trrojan/timer.h"

#include "trrojan/stream/worker_thread.h"

#include <iostream>


#define _TRROJANSTREAM_DEFINE_FACTOR(f)                                        \
const std::string trrojan::stream::stream_benchmark::factor_##f(#f)

_TRROJANSTREAM_DEFINE_FACTOR(access_pattern);
_TRROJANSTREAM_DEFINE_FACTOR(iterations);
_TRROJANSTREAM_DEFINE_FACTOR(problem_size);
_TRROJANSTREAM_DEFINE_FACTOR(scalar);
_TRROJANSTREAM_DEFINE_FACTOR(scalar_type);
_TRROJANSTREAM_DEFINE_FACTOR(task_type);
_TRROJANSTREAM_DEFINE_FACTOR(threads);

#undef _TRROJANSTREAM_DEFINE_FACTOR


/*
 * trrojan::stream::stream_benchmark::stream_benchmark
 */
trrojan::stream::stream_benchmark::stream_benchmark(void)
        : trrojan::benchmark_base("stream") {
    // If no scalar type is specfieid, use float.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_scalar_type, scalar_type_traits<scalar_type::float32>::name()));

    // If no scalar is specified, use a magic number.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_scalar, 42));

    // If no access pattern is specified, test all.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_access_pattern, { ap_traits<access_pattern::contiguous>::name(),
        ap_traits<access_pattern::interleaved>::name() }));

    // If no number of iterations is specified, use a magic number.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_iterations, 10));

    // If no number of threads is specifed, use all possible values up
    // to the number of logical processors in the system.
    auto lc = system_factors::instance().logical_cores().as<uint32_t>();
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_threads, { 1u, lc }));

    // If no problem size is given, test all all of them.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_problem_size,
        worker_thread::problem_sizes::to_vector()));

    // Enable all tasks by default.
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_task_type, { task_type_traits<task_type::add>::name(),
        task_type_traits<task_type::copy>::name(),
        task_type_traits<task_type::scale>::name(),
        task_type_traits<task_type::triad>::name() }));
}


/*
 * trrojan::stream::stream_benchmark::~stream_benchmark
 */
trrojan::stream::stream_benchmark::~stream_benchmark(void) { }


/*
 * trrojan::stream::stream_benchmark::run
 */
trrojan::result_set trrojan::stream::stream_benchmark::run(
        const configuration_set& configs) {
    std::vector<std::string> changed;
    result_set retval;

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
            retval.push_back(std::move(this->run(c)));
            return true;
        } catch (const std::exception& ex) {
            log::instance().write_line(ex);
            return false;
        }
    });

    return retval;
}


/*
 * trrojan::stream::stream_benchmark::run
 */
trrojan::result trrojan::stream::stream_benchmark::run(
        const configuration& config) {
    trrojan::result retval;

    // TODO: remove hack
    //std::cout << "here" << std::endl;
    auto p = stream_benchmark::to_problem(config);
    auto t = worker_thread::create(p);
    worker_thread::join(t.begin(), t.end());

    // TODO: retrieve and merge results

    return retval;
}


/*
 * trrojan::stream::stream_benchmark::to_problem
 */
trrojan::stream::problem::pointer_type
trrojan::stream::stream_benchmark::to_problem(const configuration& c) {
    assert(c.contains(factor_scalar_type));
    assert(c.contains(factor_scalar));
    assert(c.contains(factor_access_pattern));

    auto scalar = parse_scalar_type(*c.find(factor_scalar_type));
    auto value = c.find(factor_scalar)->value();
    auto task = parse_task_type(*c.find(factor_task_type));
    auto pattern = parse_access_pattern(*c.find(factor_access_pattern));
    auto size = c.get(factor_problem_size, problem::default_problem_size);
    auto iterations = c.get(factor_iterations, problem::default_iterations);
    auto parallelism = c.get(factor_threads, 1);

    return std::make_shared<problem>(scalar, value, task, pattern, size,
        iterations, parallelism);
}
