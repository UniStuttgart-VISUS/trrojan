/// <copyright file="stream_benchmark.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::stream::stream_benchmark::collect_results
 */
template<class I>
trrojan::result trrojan::stream::stream_benchmark::collect_results(
        const configuration& config, problem::pointer_type problem,
        I begin, I end) {
    typedef std::numeric_limits<timer::millis_type> timer_limits;

    assert(problem != nullptr);
    auto cntResults = problem->iterations();
    auto cntThreads = std::distance(begin, end);
    std::vector<std::string> names = { result_name_range_start,
        result_name_range_total, result_name_time_maximum,
        result_name_time_average, result_name_time_minimum,
        result_name_rate_minimum, result_name_rate_average,
        result_name_rate_maximum, result_name_rate_total,
        result_name_rate_aggregated };
    worker_thread::results_type results;

    // Get the results for all iterations of all threads. The array 'results'
    // will hold the data in the following order: { t1/it1, t1/it2, ..., t1/itn,
    // t2/it1, t2/it2, ..., t2/it1, t3/it1, ... }
    results.reserve(cntResults * cntThreads);
    cntThreads = 0;
    for (auto it = begin; it != end; ++it) {
        (**it).copy_results(std::back_inserter(results));
        ++cntThreads;
    }
    assert(results.size() == cntThreads * cntResults);

    //for (size_t i = 0; i < cntThreads; ++i) {
    //    names.emplace_back(result_name_rate + std::to_string(i));
    //}

    auto retval = std::make_shared<basic_result>(
        config,
        //std::move(configuration::with_system_factors(config)),
        std::move(names));

    // Combine the results per iteration.
    for (size_t i = 0; i < cntResults; ++i) {
        auto accesses = results[i].memory_accesses; // Consistent over threads!
        assert(accesses >= 2);
        assert(accesses <= 3);
        auto minStart = (timer_limits::max)();
        auto maxStart = (timer_limits::min)();
        auto maxStop = (timer_limits::min)();
        auto minTime = (timer_limits::max)();
        auto maxTime = (timer_limits::min)();
        auto sumTime = static_cast<timer::millis_type>(0);
        auto sumRate = 0.0;

        for (size_t t = 0; t < cntThreads; ++t) {
            auto idx = (t * cntResults) + i;
            assert(results[idx].memory_accesses == accesses);
            auto start = timer::millis_since_epoch(results[idx].start);
            auto time = results[idx].time;
            auto stop = start + time;

            // Compute range of start values, because this variance is
            // interesting.
            if (start < minStart) {
                minStart = start;
            }
            if (start > maxStart) {
                maxStart = start;
            }

            // The last end is also interesting to compute the total range.
            if (stop > maxStop) {
                maxStop = stop;
            }

            if (time < minTime) {
                minTime = time;
            }
            if (time > maxTime) {
                maxTime = time;
            }

            sumTime += time;
            sumRate += problem->calc_thread_mb_per_s(time, accesses);
        }

        auto rangeStart = maxStart - minStart;
        auto rangeTotal = maxStop - minStart;
        auto avgTime = (sumTime / cntThreads);
        auto minRate = problem->calc_thread_mb_per_s(maxTime, accesses);
        auto avgRate = (sumRate / cntThreads);
        auto maxRate = problem->calc_thread_mb_per_s(minTime, accesses);
        auto totalRate = problem->calc_thread_mb_per_s(rangeTotal, accesses);

#if (defined(DEBUG) || defined(_DEBUG))
        std::cout << "iteration " << i
            << ": start range = " << rangeStart
            << ", total range = " << rangeTotal
            << ", maximum time = " << maxTime
            << ", average time = " << avgTime
            << ", minimum time = " << minTime
            << ", (sum time = " << sumTime << ")"
            << ", maximum rate = " << maxRate
            << ", average rate = " << avgRate
            << ", minimum rate = " << minRate
            << ", total rate = " << rangeTotal
            << ", summed rate = " << sumRate
            << std::endl;
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

        retval->add({ rangeStart, rangeTotal, maxTime, avgTime,
            minTime, minRate, avgRate, maxRate, totalRate, sumRate });
    }

    return std::dynamic_pointer_cast<result::element_type>(retval);
}
