/// <copyright file="stream_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/stream_benchmark.h"

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/system_factors.h"


/*
 * trrojan::stream::stream_benchmark::access_pattern_contiguous
 */
const std::string trrojan::stream::stream_benchmark::access_pattern_contiguous(
    "contiguous");


/*
 * trrojan::stream::stream_benchmark::access_pattern_interleaved
 */
const std::string trrojan::stream::stream_benchmark::access_pattern_interleaved(
    "interleaved");


/*
 * trrojan::stream::stream_benchmark::~stream_benchmark
 */
trrojan::stream::stream_benchmark::~stream_benchmark(void) { }


/*
 * trrojan::stream::stream_benchmark::run
 */
trrojan::result_set trrojan::stream::stream_benchmark::run(
        const configuration_set& configs) {
    this->check_required_factors(configs);

    auto cs = configs;
    if (!cs.contains_factor("scalar")) {
        // If no scalar is specified, use a magic number.
        cs.add_factor(factor::from_manifestations("scalar", 42));
    }
    if (!cs.contains_factor("access_pattern")) {
        // If no access pattern is specified, test all.
        cs.add_factor(factor::from_manifestations("access_pattern",
            { access_pattern_contiguous, access_pattern_interleaved }));
    }
    if (!cs.contains_factor("iterations")) {
        // If no number of iterations is specified, use a magic number.
        cs.add_factor(factor::from_manifestations("iterations", 10));
    }
    if (!cs.contains_factor("threads")) {
        // If no number of threads is specifed, use all possible values up
        // to the number of logical processors in the system.
        auto flc = system_factors::instance().logical_cores();
        auto lc = static_cast<int>(flc.get_uint32());
        cs.add_factor(factor::from_manifestations("threads", { 1, lc }));
    }

    // Problem size

    return result_set();
}
