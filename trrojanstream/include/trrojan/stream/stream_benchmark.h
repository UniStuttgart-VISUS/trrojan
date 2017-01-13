/// <copyright file="stream_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"

#include <algorithm>
#include <atomic>
#include <memory>

#include "trrojan/enum_parse_helper.h"

#include "trrojan/stream/export.h"
#include "trrojan/stream/problem.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// The implementation of the memory streaming benchmark.
    /// </summary>
    /// <remarks>
    /// <para>The implementation of this benchmark is closely related to John
    /// McCalpin's STREAM benchmark (http://www.cs.virginia.edu/stream/), but
    /// uses a different multi-threading model, which hopefully minimises the
    /// variance between the starting time of different threads compared to an
    /// OpenMP-based implementation, which is also dependent on the runtime
    /// used for the test.</para>
    /// <para>Most of the benchmark implementation is located in
    /// <see cref="trrojan::stream::worker_thread" />. Please refer to this
    /// class for implementation details of the benchmark.</para>
    /// </remarks>
    class TRROJANSTREAM_API stream_benchmark : public trrojan::benchmark_base {

    public:

        static const std::string factor_access_pattern;
        static const std::string factor_iterations;
        static const std::string factor_problem_size;
        static const std::string factor_scalar;
        static const std::string factor_scalar_type;
        static const std::string factor_task_type;
        static const std::string factor_threads;

        stream_benchmark(void);

        virtual ~stream_benchmark(void);

        virtual trrojan::result_set run(const configuration_set& configs);

        virtual trrojan::result run(const configuration& config);

    private:

        template<access_pattern A>
        using ap_traits = trrojan::stream::access_pattern_traits<A, 0>;

        static inline access_pattern parse_access_pattern(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<access_pattern, access_pattern_list_t,
                ap_traits> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(access_pattern_list(), value);
        }

        static inline scalar_type parse_scalar_type(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<scalar_type, scalar_type_list_t,
                scalar_type_traits> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(scalar_type_list(), value);
        }

        static inline task_type parse_task_type(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<task_type, task_type_list_t,
                task_type_traits> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(task_type_list(), value);
        }

        static trrojan::stream::problem::pointer_type to_problem(
            const configuration& c);
    };

}
}
