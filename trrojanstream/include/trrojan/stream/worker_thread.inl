#include "worker_thread.h"
/// <copyright file="worker_thread.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>



/*
 * trrojan::stream::worker_thread::join
 */
template<class I>
static void trrojan::stream::worker_thread::join(I begin, I end) {
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
            //TODO: trrojan::log::write_line()
            return false;
        }
    }
    /* No problem found at this point. */

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
            //TODO: trrojan::log::write_line()
            return false;
    }
}
