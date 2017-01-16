/// <copyright file="worker_thread.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright (C) 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>



/*
 * trrojan::stream::worker_thread::join
 */
template<class I>
void trrojan::stream::worker_thread::join(I begin, I end) {
    trrojan::log::instance().write(trrojan::log_level::verbose,
        "Waiting for %u worker thread(s) to exit.\n",
        std::distance(begin, end));

#ifdef _WIN32
    std::vector<HANDLE> handles;

    I it = begin;
    while (it != end) {
        // Fill at most the supported number of handles into the array and
        // wait for those handles using one API call.
        for (; (it != end) && (handles.size() < MAXIMUM_WAIT_OBJECTS); ++it) {
            handles.push_back((**it).hThread);
        }

        // Wait for the handles.
        auto status = ::WaitForMultipleObjects(static_cast<DWORD>(handles.size()),
            handles.data(), TRUE, INFINITE);
        if (status != WAIT_OBJECT_0) {
            std::error_code ec(::GetLastError(), std::system_category());
            throw std::system_error(ec, "Failed to join worker threads.");
        }

        // Clear for next iteration.
        handles.clear();
    }

#else /* _WIN32 */
    for (auto it = begin; it != end; ++it) {
        if (::pthread_join((**it).hThread, nullptr) != 0) {
            std::error_code ec(errno, std::system_category());
            throw std::system_error(ec, "Failed to join worker thread.");
        }
    }
#endif /* _WIN32 */
}


/*
 * trrojan::stream::worker_thread::verify
 */
template<class S, trrojan::stream::task_type T>
bool trrojan::stream::worker_thread::verify(const S *a, const S *b, const S *c,
        const S s, const size_t cnt) {
    S expected;

    for (size_t i = 0; i < cnt; ++i) {
        step<1, scalar_reverse_traits<S>::type, T>::apply(a + i, b + i,
            &expected, s, 0);
        if (expected != c[i]) {
            trrojan::log::instance().write(trrojan::log_level::warning,
                "Verification of stream results failed for item %u: found %s, "
                "but expected %s.\n", i, std::to_string(c[i]).c_str(),
                std::to_string(expected).c_str());
            return false;
        }
    }
    /* No problem found at this point. */

    trrojan::log::instance().write(trrojan::log_level::information,
        "Verification of %u stream results succeeded.\n", cnt);
    return true;
}


/*
 * trrojan::stream::worker_thread::verify
 */
template<class S> 
bool trrojan::stream::worker_thread::verify(const S *a, const S *b, const S *c,
        const S s, const size_t cnt, const task_type task) {
    switch (task) {
        case task_type::add:
            return worker_thread::verify<S, task_type::add>(a, b, c, s, cnt);
        case task_type::copy:
            return worker_thread::verify<S, task_type::copy>(a, b, c, s, cnt);
        case task_type::scale:
            return worker_thread::verify<S, task_type::scale>(a, b, c, s, cnt);
        case task_type::triad:
            return worker_thread::verify<S, task_type::triad>(a, b, c, s, cnt);
        default:
            throw std::logic_error("No verification is possible for the given "
                "task.");
    }
}


/*
 * trrojan::stream::worker_thread::dispatch
 */
template<trrojan::stream::scalar_type S,
    trrojan::stream::worker_thread::problem_size_type P,
    trrojan::stream::access_pattern A,
    trrojan::stream::task_type T,
    trrojan::stream::task_type... Ts>
void trrojan::stream::worker_thread::dispatch(
        trrojan::stream::task_type_list_t<T, Ts...>,
        const trrojan::stream::task_type t) {
    assert(this->_problem != nullptr);
    assert(this->results.size() == this->_problem->iterations() + 1);

    if (T == t) {
        typedef access_pattern_traits<A, P> pattern;
        typedef step<P, S, T> step;

        auto offset = pattern::offset(this->rank);
        auto a = this->_problem->a<S>() + offset;
        auto b = this->_problem->b<S>() + offset;
        auto c = this->_problem->c<S>() + offset;
        auto s = this->_problem->s<S>();
        auto o = pattern::step(this->_problem->parallelism());
        auto cnt = this->_problem->iterations();
        trrojan::timer timer;

        log::instance().write(log_level::verbose, "Worker thread %u is "
            "performing the following test: size = %u, offset = %u, "
            "step = %u, task = %i, access pattern = %i, scalar type = %i, "
            "scalar value = %i, iterations = %u\n", this->rank, P, offset, o,
            static_cast<int>(T), static_cast<int>(A), static_cast<int>(S),
            s, cnt);

        for (size_t i = 0; i <= cnt; ++i) {
            auto& result = this->results[i];
            result.memory_accesses = task_type_traits<T>::memory_accesses;
            // Note: we assign 'memory_accesses' before entering the spin lock,
            // because it enforces that 'result' is used before the splin lock,
            // which forces the compiler to initialise the local variable at
            // the point where it is in the code. Otherwise, some compilers
            // reorder the operations, because 'result' is not used before the
            // spin lock was passed.
            this->synchronise(i);
            result.start = timer.start();
            step::apply(a, b, c, s, o);
            result.time = timer.elapsed_millis();
            // std::cout << "Iteration " << i << ", worker " << this->rank << ": " << this->_problem->calc_mb_per_s(result.time) << " MB/s" << std::endl;
        }

    } else {
        this->dispatch<S, P, A>(
            trrojan::stream::task_type_list_t<Ts...>(),
            t);
    }
}
