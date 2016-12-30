/// <copyright file="stream_benchmark.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/stream_benchmark.h"

#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/system_factors.h"

#if 0
/*
* worker_thread<T>::create
*/
template<class T>
typename worker_thread<T>::pointer_type worker_thread<T>::create(
    scenario_type scenario, const size_type rank,
    barrier_type barrier, const uint64_t affinity, const uint16_t group) {
    worker_thread::pointer_type retval(new worker_thread());
    retval->rank = rank;
    retval->scenario = scenario;
    retval->barrier = (barrier != nullptr) ? barrier : make_barrier();

    // Note: we make one additional run for "prewarming" the chaches. This
    // result is later discarded, but must nevertheless be stored.
    retval->times[test_result::test_name_add].resize(scenario->iterations() + 1);
    retval->times[test_result::test_name_copy].resize(scenario->iterations() + 1);
    retval->times[test_result::test_name_scale].resize(scenario->iterations() + 1);
    retval->times[test_result::test_name_triad].resize(scenario->iterations() + 1);

#ifdef _WIN32
    /* Create suspended thread. */
    retval->hThread = ::CreateThread(nullptr, 0, worker_thread::thunk,
        retval.get(), CREATE_SUSPENDED, nullptr);
    if (retval->hThread == NULL) {
        throw std::exception("Failed to create worker thread.");
    }

    /* Set affinity. */
    {
        GROUP_AFFINITY ga;
        ::ZeroMemory(&ga, sizeof(ga));
        if (affinity != 0) {
            ga.Group = group;
            ga.Mask = affinity;
        } else {
            uint64_t groupSize = sizeof(ga.Mask) * 8;
            uint64_t r = rank;
            ga.Group = static_cast<WORD>(r / groupSize);
            r %= groupSize;
            ga.Mask = static_cast<uint64_t>(1) << r;
        }

        auto status = ::SetThreadGroupAffinity(retval->hThread, &ga, nullptr);
        if (!status) {
            throw std::exception("Setting thread group affinity failed.");
        }
    }

    /* Give maximum priority to thread. */
    if (!::SetThreadPriority(retval->hThread, THREAD_PRIORITY_TIME_CRITICAL)) {
        throw std::exception("Setting thread priority failed.");
    }

    /* Ensure that process has maximum priority, too. */
    if (!::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
        throw std::exception("Setting process priority failed.");
    }

    /* Start thread. */
    {
        auto status = ::ResumeThread(retval->hThread);
        if (status == static_cast<DWORD>(-1)) {
            throw std::exception("Resuming worker thread failed.");
        }
    }
#else /* _WIN32 */
    // http://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
#error "Implementation missing!"
#endif /* _WIN32 */

    return retval;
}
#endif

#if 0
/*
 * worker_thread<T>::thunk
 */
template<class T> DWORD worker_thread<T>::thunk(void * param) {
    auto that = static_cast<worker_thread *>(param);

    // Get local of all arrays and values we access frequently to bypass
    // the indirection introduced by the shared pointer.
    auto a = that->scenario->a();
    auto b = that->scenario->b();
    auto c = that->scenario->c();
    auto scalar = that->scenario->scalar();
    auto timesAdd = that->times[test_result::test_name_add].data();
    auto timesCopy = that->times[test_result::test_name_copy].data();
    auto timesScale = that->times[test_result::test_name_scale].data();
    auto timesTriad = that->times[test_result::test_name_triad].data();
    const size_type r = that->scenario->iterations() + 1;
    const size_type o = that->scenario->offset(that->rank);
    const size_type s = that->scenario->step_size();
    const size_type e = that->scenario->end(that->rank);
    int barrierId = 0;

    // Ensure that output buffer has expected size.
    assert(that->times[test_result::test_name_add].size() == r);
    assert(that->times[test_result::test_name_copy].size() == r);
    assert(that->times[test_result::test_name_scale].size() == r);
    assert(that->times[test_result::test_name_triad].size() == r);
    // Ensure that step size is positive.
    assert(s > 0);
    // Ensure that start position is within valid range.
    assert(o >= 0);
    assert(o < that->scenario->problem_size());
    // Ensure that end is within valid range (index after last element).
    assert(e <= that->scenario->problem_size());

    for (size_t i = 0; i < r; ++i) {

        /* Copy test. */
#if (defined(DEBUG) || defined(_DEBUG))
        size_t __dbgCnt = 0;
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        that->synchronise(barrierId++);
        timesCopy[i].first = system_information::get_timer_seconds();
        for (size_t j = o; j < e; j += s) {
            c[j] = a[j];
#if (defined(DEBUG) || defined(_DEBUG))
            ++__dbgCnt;
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        }
        timesCopy[i].second = system_information::get_timer_seconds();
#if (defined(DEBUG) || defined(_DEBUG))
        assert(__dbgCnt == that->problem_size());
#endif /* (defined(DEBUG) || defined(_DEBUG)) */

        /* Scaling test. */
        that->synchronise(barrierId++);
        timesScale[i].first = system_information::get_timer_seconds();
        for (size_t j = o; j < e; j += s) {
            b[j] = scalar * c[j];
        }
        timesScale[i].second = system_information::get_timer_seconds();

        /* Adding test. */
        that->synchronise(barrierId++);
        timesAdd[i].first = system_information::get_timer_seconds();
        for (size_t j = o; j < e; j += s) {
            c[j] = a[j] + b[j];
        }
        timesAdd[i].second = system_information::get_timer_seconds();

        /* Triad test. */
        that->synchronise(barrierId++);
        timesTriad[i].first = system_information::get_timer_seconds();
        for (size_t j = o; j < e; j += s) {
            a[j] = b[j] + scalar * c[j];
        }
        timesTriad[i].second = system_information::get_timer_seconds();
    }

    /* Remove the first iteration for each test. */
    for (auto& t : that->times) {
        t.second.erase(t.second.begin());
        assert(t.second.size() == that->scenario->iterations());
    }

#if 0
    for (k = 1; k<NTIMES; k++) /* note -- skip first iteration */
    {
        for (j = 0; j<4; j++) {
            avgtime[j] = avgtime[j] + times[j][k];
            mintime[j] = MIN(mintime[j], times[j][k]);
            maxtime[j] = MAX(maxtime[j], times[j][k]);
        }
    }

    printf("Function      Rate (MB/s)   Avg time     Min time     Max time\n");
    for (j = 0; j<4; j++) {
        avgtime[j] = avgtime[j] / (double)(NTIMES - 1);

        printf("%s%11.4f  %11.4f  %11.4f  %11.4f\n", label[j],
            1.0E-06 * bytes[j] / mintime[j],
            avgtime[j],
            mintime[j],
            maxtime[j]);
    }
#endif

    //std::cout << "Worker is running." << std::endl;

    return 0;
}
#endif

#if 0
/*
* worker_thread<T>::synchronise
*/
template<class T> void worker_thread<T>::synchronise(const int barrierId) {
    // Implementation of repeated barrier as in
    // http://stackoverflow.com/questions/24205226/how-to-implement-a-re-usable-thread-barrier-with-stdatomic
    assert(INT_MAX / this->scenario->number_of_threads() > barrierId);
    auto& barrier = *this->barrier;
    int expected = (barrierId + 1) * static_cast<int>(
        this->scenario->number_of_threads());
    ++barrier;
    while (barrier - barrierId < 0);    // TODO: Should yield if too many threads.
}
#endif



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
        cs.add_factor(factor::from_range("threads", 1, lc, lc - 1));
    }

    // Problem size

    return result_set();
}
