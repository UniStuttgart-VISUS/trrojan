/// <copyright file="plugin.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/worker_thread.h"


/*
 * trrojan::stream::worker_thread::create
 */
trrojan::stream::worker_thread::pointer_type
trrojan::stream::worker_thread::create(problem_type problem,
        barrier_type barrier, const rank_type rank,
        const uint64_t affinity_mask, const uint16_t affinity_group) {
    auto retval = std::make_shared<worker_thread>();
    retval->start(problem, barrier, rank, affinity_mask, affinity_group);
    return retval;
}


/*
 * trrojan::stream::worker_thread::create
 */
std::vector<trrojan::stream::worker_thread::pointer_type>
trrojan::stream::worker_thread::create(problem_type problem) {
    if (problem == nullptr) {
        throw std::invalid_argument("The problem must not be null.");
    }

    auto barrier = worker_thread::make_barrier(problem->parallelism());

    std::vector<pointer_type> retval;
    retval.reserve(problem->parallelism());

    for (size_t i = 0; i < problem->parallelism(); ++i) {
        retval.push_back(worker_thread::create(problem, barrier, i));
    }

    return retval;
}


/*
 * trrojan::stream::worker_thread::start
 */
void trrojan::stream::worker_thread::start(problem_type problem,
        barrier_type barrier, const rank_type rank,
        const uint64_t affinity_mask, const uint16_t affinity_group) {
    if (problem == nullptr) {
        throw std::invalid_argument("The problem must not be null.");
    }
    if (barrier == nullptr) {
        throw std::invalid_argument("The barrier must not be null.");
    }
    if (this->hThread != static_cast<handle_type>(0)) {
        throw std::logic_error("A worker thread can only be started while it "
            "is not yet running.");
    }

    this->barrier = barrier;
    this->_problem = problem;
    this->rank = rank;

    /* Allocate the result set before starting the thread. */
    this->results_lock.lock();
    this->results.resize(this->_problem->iterations() + 1);
    this->results_lock.unlock();

    trrojan::log::instance().write(log_level::verbose, "Starting worker "
        "thread with rank {}...\n", this->rank);

#ifdef _WIN32
    /* Create suspended thread. */
    this->hThread = ::CreateThread(nullptr, 0, worker_thread::thunk, this,
            CREATE_SUSPENDED, nullptr);
    if (this->hThread == NULL) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Failed to spawn suspended worker thread.");
    }

    /* Set affinity. */
    {
        GROUP_AFFINITY ga;
        ::ZeroMemory(&ga, sizeof(ga));
        if (affinity_mask != 0) {
            ga.Group = affinity_group;
            ga.Mask = affinity_mask;
        } else {
            uint64_t groupSize = sizeof(ga.Mask) * CHAR_BIT;
            uint64_t r = rank;
            ga.Group = static_cast<WORD>(r / groupSize);
            r %= groupSize;
            ga.Mask = static_cast<uint64_t>(1) << r;
        }

        auto status = ::SetThreadGroupAffinity(this->hThread, &ga, nullptr);
        if (!status) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Setting thread affinity failed.");
        }
    }

    /* Give maximum priority to thread. */
    if (!::SetThreadPriority(this->hThread, THREAD_PRIORITY_TIME_CRITICAL)) {
        std::error_code ec(::GetLastError(), std::system_category());
        throw std::system_error(ec, "Setting thread priority failed.");
    }

    /* Ensure that process has maximum priority, too. */
    if (!::SetPriorityClass(::GetCurrentProcess(), REALTIME_PRIORITY_CLASS)) {
        throw std::exception("Setting process priority failed.");
    }

    /* Start thread. */
    {
        auto status = ::ResumeThread(this->hThread);
        if (status == static_cast<DWORD>(-1)) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Resuming worker thread failed.");
        }
    }

#else /* _WIN32 */
    pthread_attr_t attribs;
    ::pthread_attr_init(&attribs);
    ::pthread_attr_setscope(&attribs, PTHREAD_SCOPE_SYSTEM);
    ::pthread_attr_setdetachstate(&attribs, PTHREAD_CREATE_JOINABLE);

    if (::pthread_create(&this->hThread, &attribs, worker_thread::thunk,
            static_cast<void *>(this)) != 0) {
        std::error_code ec(errno, std::system_category());
        ::pthread_attr_destroy(&attribs);
        throw std::system_error(ec, "Failed to spawn worker thread.");
    } else {
        ::pthread_attr_destroy(&attribs);
    }

    // TODO: priority
    // TODO: affinity

    log::instance().write(log_level::error, "TODO: IMPLEMENTATION OF AFFINITY MISSING!\n");

    // http://eli.thegreenplace.net/2016/c11-threads-affinity-and-hyperthreading/
    /*
    // Create a cpu_set_t object representing a set of CPUs. Clear it and mark
    // only CPU i as set.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    int rc = pthread_setaffinity_np(threads[i].native_handle(),
    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
    std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
    */

//    throw 1;

#endif /* _WIN32 */
}


/*
 * trrojan::stream::worker_thread::thunk
 */
#ifdef _WIN32
DWORD WINAPI trrojan::stream::worker_thread::thunk(void *param) {
#else /* _WIN32 */
void *trrojan::stream::worker_thread::thunk(void *param) {
#endif /* _WIN32*/
    auto that = static_cast<worker_thread *>(param);
    assert(that != nullptr);
    assert(that->_problem != nullptr);

    that->results_lock.lock();
    that->dispatch(scalar_type_list(),
        that->_problem->scalar_type(),
        that->_problem->access_pattern(),
        that->_problem->size(),
        that->_problem->task_type());
    that->results_lock.unlock();

    return 0;
}


/*
 * trrojan::stream::worker_thread::synchronise
 */
void trrojan::stream::worker_thread::synchronise(const int barrierId) {
    // Implementation of repeated barrier as in
    // http://stackoverflow.com/questions/24205226/how-to-implement-a-re-usable-thread-barrier-with-stdatomic
    assert(this->barrier != nullptr);
    assert(this->_problem != nullptr);
    assert(INT_MAX / this->_problem->parallelism() > barrierId);
    auto& barrier = *this->barrier;
    auto expected = static_cast<int>(this->_problem->parallelism());
    expected *= (barrierId + 1);
    ++barrier;
    while (barrier.load() - barrierId < 0);    // TODO: Should yield if too many threads?
}

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
