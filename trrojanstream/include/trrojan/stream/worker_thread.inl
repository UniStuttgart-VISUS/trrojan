/// <copyright file="worker_thread.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright (C) 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>



/*
 * trrojan::stream::worker_thread::join
 */
template<class I>
void trrojan::stream::worker_thread::join(I begin, I end) {
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
    if (T == t) {
        std::cout << "task type " << (int) T << " selected." << std::endl;
        typedef step<P, S, T> step;

        trrojan::timer timer;
        this->synchronise(0);   // TODO: repeat
        timer.start();
        step::apply(this->_problem->a<S>(), this->_problem->b<S>(),
            this->_problem->c<S>(), this->_problem->s<S>(),
            access_pattern_traits<A, P>::offset);
        auto elapsed = timer.elapsed_millis();
        elapsed /= trrojan::constants<decltype(elapsed)>::millis_per_second;
        auto gigs = (double)this->_problem->size_in_bytes() / trrojan::constants<decltype(elapsed)>::bytes_per_gigabyte;
        std::cout << "gigs " << gigs << std::endl;

    } else {
        this->dispatch<S, P, A>(
            trrojan::stream::task_type_list_t<Ts...>(),
            t);
    }
}


#if 0
/*
 * trrojan::stream::worker_thread::dispatch<T>::invoke
 */
template<trrojan::stream::scalar_type T>
void trrojan::stream::worker_thread::dispatch<T>::invoke(worker_thread *that) {
    typedef typename scalar_type_traits<T>::type type;
    assert(that != nullptr);
    assert(that->barrier != nullptr);
    assert(that->_problem != nullptr);

    // Get local of all arrays and values we access frequently to bypass
    // the indirection introduced by the shared pointer and to prevent the
    // overhead induced by casts etc.
    auto a = that->_problem->a<type>();
    auto b = that->_problem->b<type>();
    auto c = that->_problem->c<type>();
    auto& barrier = *that->barrier;
    auto parallelism = that->_problem->parallelism();
    //auto scalar = that->scenario->scalar();
    //auto timesAdd = that->times[test_result::test_name_add].data();
    //auto timesCopy = that->times[test_result::test_name_copy].data();
    //auto timesScale = that->times[test_result::test_name_scale].data();
    //auto timesTriad = that->times[test_result::test_name_triad].data();
    //const size_type r = that->scenario->iterations() + 1;
    int barrierId = 0;

    size_t r = 10;  // TODO

    typedef step<1000000, T, task_type::copy> step_type;


    for (size_t i = 0; i < r; ++i) {
        that->synchronise(barrierId++);
        //timesCopy[i].first = system_information::get_timer_seconds();
        
        //timesCopy[i].second = system_information::get_timer_seconds();
    }
}
#endif
