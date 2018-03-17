/// <copyright file="stream_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <iterator>
#include <limits>
#include <memory>

#include "trrojan/enum_parse_helper.h"
#include "trrojan/timer.h"

#include "trrojan/stream/export.h"
#include "trrojan/stream/problem.h"
#include "trrojan/stream/worker_thread.h"


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
    /// <para>The benchmark supports the following
    /// <see cref="trrojan::factor" />s, which all of have reasonable default
    /// values:</para>
    /// <list type="bullet">
    /// <item>
    /// <term>access_pattern</term>
    /// <description>The memory access pattern if using more than one thread.
    /// See documentation of <see cref="trrojan::stream::access_pattern" /> for
    /// details on the respective behaviour.</description>
    /// </item>
    /// <item>
    /// <term>threads</term>
    /// <description>The number of threads to use simultaneously. Note that at
    /// must one thread per logical core must be started. The problem size will
    /// be scaled by the number of threads.</description>
    /// </item>
    /// <item>
    /// <term>iterations</term>
    /// <description>The number of iterations a single test configuration will
    /// be repeated. The <see cref="trrojan::stream::worker_thread" /> will add
    /// one warm-up iteration to this number, which will not be reported in the
    /// results.</description>
    /// </item>
    /// <item>
    /// <term>problem_size</term>
    /// <description>The problem size in number of items to be processed.
    /// </description>
    /// </item>
    /// <item>
    /// <term>scalar</term>
    /// <description>The scalar value used for the tasks
    /// <see cref="trrojan::stream::task_type::scale" /> and
    /// <see cref="trrojan::stream::task_type::triad" />. The string
    /// representation of <see cref="trrojan::stream::task_type" /> must be
    /// used for this factor.</description>
    /// </item>
    /// <item>
    /// <term>scalar_type</term>
    /// <description>The type of a scalar in the tests. The string representation
    /// of <see cref="trrojan::stream::scalar_type" /> must be used for this
    /// factor.</description>
    /// </item>
    /// <item>
    /// <term>task_type</term>
    /// <description>The task to be performed. The string representation
    /// of <see cref="trrojan::stream::task_type" /> must be used for this
    /// factor.</description>
    /// </item>
    /// </list>
    /// </remarks>
    class TRROJANSTREAM_API stream_benchmark : public trrojan::benchmark_base {

    public:

        typedef benchmark_base::on_result_callback on_result_callback;

        static const std::string factor_access_pattern;
        static const std::string factor_iterations;
        static const std::string factor_problem_size;
        static const std::string factor_scalar;
        static const std::string factor_scalar_type;
        static const std::string factor_task_type;
        static const std::string factor_threads;

        static const std::string result_name_rate_aggregated;
        static const std::string result_name_rate_average;
        static const std::string result_name_rate_maximum;
        static const std::string result_name_rate_minimum;
        static const std::string result_name_rate_total;
        static const std::string result_name_range_start;
        static const std::string result_name_range_total;
        static const std::string result_name_time_average;
        static const std::string result_name_time_maximum;
        static const std::string result_name_time_minimum;
        static const std::string result_name_time_slowest;

        stream_benchmark(void);

        virtual ~stream_benchmark(void);

        virtual size_t run(const configuration_set& configs,
            const on_result_callback& callback,
            const cool_down& coolDown);

        virtual trrojan::result run(const configuration& config);

    private:

        template<access_pattern A>
        using ap_traits = trrojan::stream::access_pattern_traits<A, 0>;

        static inline access_pattern parse_access_pattern(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<access_pattern, ap_traits,
                access_pattern_list_t> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(access_pattern_list(), value);
        }

        static inline scalar_type parse_scalar_type(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<scalar_type, scalar_type_traits,
                scalar_type_list_t> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(scalar_type_list(), value);
        }

        static inline task_type parse_task_type(
                const trrojan::named_variant& s) {
            typedef enum_parse_helper<task_type, task_type_traits,
                task_type_list_t> parser;
            auto value = s.value().as<std::string>();
            return parser::parse(task_type_list(), value);
        }

        static trrojan::stream::problem::pointer_type to_problem(
            const configuration& c);

        template<class I>trrojan::result collect_results(
            const configuration& config, problem::pointer_type problem,
            I begin, I end);
    };

}
}

#include "trrojan/stream/stream_benchmark.inl"
