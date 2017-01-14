/// <copyright file="worker_thread.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright (C) 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <atomic>
#include <cinttypes>
#include <climits>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else /* _WIN32 */
#include <pthread.h>
#endif /* _WIN32 */

#include "trrojan/constants.h"
#include "trrojan/index_sequence.h"
#include "trrojan/log.h"
#include "trrojan/timer.h"

#include "trrojan/stream/access_pattern.h"
#include "trrojan/stream/export.h"
#include "trrojan/stream/problem.h"
#include "trrojan/stream/scalar_type.h"
#include "trrojan/stream/task_type.h"


#if defined(_MSC_VER)
#define TRROJANSTREAM_FORCE_INLINE __forceinline
#elif defined (__GNUC__)
#define TRROJANSTREAM_FORCE_INLINE __attribute__((always_inline)) inline
#else /* defined(_MSC_VER) */
#define TRROJANSTREAM_FORCE_INLINE inline
#endif /* defined(_MSC_VER) */


namespace trrojan {
namespace stream {

    /// <summary>
    /// A worker thread performing the benchmarking.
    /// </summary>
    /// <remarks>
    /// </remarks>
    class TRROJANSTREAM_API worker_thread {

    public:

        /// <summary>
        /// The results of a single iteration of a single test.
        /// </summary>
        struct iteration_result {

            /// <summary>
            /// The start time of the specific run.
            /// </summary>
            trrojan::timer::value_type start;

            /// <summary>
            /// The time the test run took (in milliseconds).
            /// </summary>
            trrojan::timer::millis_type time;
        };

        /// <summary>
        /// A spin lock used as a barrier to ensure simultaneous memory access.
        /// </summary>
        typedef std::shared_ptr<std::atomic<int>> barrier_type;

        /// <summary>
        /// A pointer to a <see cref="worker_thread" />.
        /// </summary>
        typedef std::shared_ptr<worker_thread> pointer_type;

        /// <summary>
        /// The type to specify problem sizes.
        /// </summary>
        typedef std::uint64_t problem_size_type;

        /// <summary>
        /// The list of available problem sizes.
        /// </summary>
        typedef trrojan::integer_sequence<problem_size_type,
            1000000, 2000000, 4000000, 8000000> problem_sizes;

        /// <summary>
        /// The type of a problem to be processed by a thread.
        /// </summary>
        typedef trrojan::stream::problem::pointer_type problem_type;

        /// <summary>
        /// A type to express a thread's rank.
        /// </summary>
        typedef std::size_t rank_type;

        /// <summary>
        /// The results of all iterations of a single thread.
        /// </summary>
        typedef std::vector<iteration_result> results_type;

        /// <summary>
        /// Creates and starts a new worker thread for the given problem.
        /// </summary>
        static pointer_type create(problem_type problem, barrier_type barrier,
            const rank_type rank, const uint64_t affinity_mask = 0,
            const uint16_t affinity_group = 0);

        /// <summary>
        /// Creates an starts the worker threads for the given problem.
        /// </summary>
        static std::vector<pointer_type> create(problem_type problem);

        /// <summary>
        /// Join all worker threads in the specified range.
        /// </summary>
        /// <param name="begin">The begin of the range of threads to join.
        /// </param>
        /// <param name="end">The begin of the range of threads to join.</param>
        /// <tparam name="I">An iterator over
        /// <see cref="worker_thread::pointer_type" />.</tparam>
        template<class I> static void join(I begin, I end);

        /// <summary>
        /// Creates a pre-initialised barriert for the given number of threads
        /// working on the same problem.
        /// </summary>
        /// <param name="parallelism"></param>
        /// <returns></returns>
        static inline barrier_type make_barrier(const size_t parallelism) {
            return std::make_shared<barrier_type::element_type>(parallelism);
        }

        /// <summary>
        /// Verifies that <paramref name="c"> holds the results of the specified
        /// task <tparamref name="T" /> executed on <paramref name="a" /> and
        /// <paramref name="b" />.
        /// </summary>
        template<class S, task_type T>
        static bool verify(const S *a, const S *b, const S *c, const S s,
            const size_t cnt);

        /// <summary>
        /// Verifies that <paramref name="c"> holds the results of the specified
        /// task <paramref name="task" /> executed on <paramref name="a" /> and
        /// <paramref name="b" />.
        /// </summary>
        template<class S>
        static bool verify(const S *a, const S *b, const S *c, const S s,
            const size_t cnt, const task_type task);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline worker_thread(void) : hThread(0), rank(0) { }

        worker_thread(const worker_thread&) = delete;

        /// <summary>
        /// Copy the results of this worker thread to the specified output
        /// iterator.
        /// </summary>
        /// <remarks>
        /// <para>Results are only available after the thread has been joined.
        /// Access to the result set is thread-safe.</para>
        /// <para>The result of the first (warm-up) run is not returned.</para>
        /// </remarks>
        template<class I> void copy_results(I oit) const {
            this->results_lock.lock();
            assert(this->results.size() > 1);
            std::copy(this->results.cbegin() + 1, this->results.cend(), oit);
            this->results_lock.unlock();
        }

        /// <summary>
        /// Starts the worker thread.
        /// </summary>
        void start(problem_type problem, barrier_type barrier,
            const rank_type rank, const uint64_t affinity_mask = 0,
            const uint16_t affinity_group = 0);

        worker_thread& operator =(const worker_thread&) = delete;

    private:

        /// <summary>
        /// The type of a native thread handle.
        /// </summary>
#ifdef _WIN32
        typedef HANDLE handle_type;
#else /* _WIN32 */
        typedef pthread_t handle_type;
#endif /* _WIN32 */

        /// <summary>
        /// This functor expands the testing loop at compile time to remove any
        /// overhead for control flow checks (which outweighs the actual tests
        /// performed by far) from the benchmark.
        /// </summary>
        /// <remarks>
        /// We are going the template way here, because compilers usually cannot
        /// unroll loops with dynamic (runtime-defined) boundaries. Therefore,
        /// the loop boundaries must be hardcoded. Using a template allows us to
        /// provide several different pre-compiled problem sizes.
        /// </remarks>
        /// <tparam name="N">The problem size (number of elements in each array
        /// per thread).</tparam>
        /// <tparam name="S">The scalar type stored in the arrays.</tparam>
        /// <tparam name="T">The type of test to be performed.</tparam>
        template<int N, scalar_type S, task_type T> struct step {
            typedef typename scalar_type_traits<S>::type scalar_type;

            // TODO: this is insanely slow; we need to make it faster ...
            //static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
            //        const scalar_type *b, scalar_type *c, const scalar_type s,
            //        const size_t o) {
            //    std::cout << N << " " << o << std::endl;
            //    step<1, S, T>::apply(a, b, c, s, o);
            //    step<N - 1, S, T>::apply(a + o, b + o, c + o, s, o);
            //}
#ifdef __GNUC___
            __attribute__((optimize("unroll-loops")))
#endif /* __GNUC___ */
            static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
                    const scalar_type *b, scalar_type *c, const scalar_type s,
                    const size_t o) {
#ifdef _MSC_VER
#pragma loop(hint_parallel(0))
#pragma loop(ivdep)
#endif /* _MSC_VER */
                for (int i = 0; i < N; ++i) {
                    step<1, S, T>::apply(a + i * o, b + i * o, c + i * o, s, o);
                }
            }
        };

        ///// <summary>
        ///// Specialisation of <see cref="step" /> which performs ten steps an
        ///// once and thus makes code generation faster.
        ///// </summary>
        //template<scalar_type S, task_type T> struct step<10, S, T> {
        //    typedef typename scalar_type_traits<S>::type scalar_type;

        //    static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
        //            const scalar_type *b, scalar_type *c, const scalar_type s,
        //            const size_t o) {
        //        step<1, S, T>::apply(a + 0 * o, b + 0 * o, c + 0 * o, s, o);
        //        step<1, S, T>::apply(a + 1 * o, b + 1 * o, c + 1 * o, s, o);
        //        step<1, S, T>::apply(a + 2 * o, b + 2 * o, c + 2 * o, s, o);
        //        step<1, S, T>::apply(a + 3 * o, b + 3 * o, c + 3 * o, s, o);
        //        step<1, S, T>::apply(a + 4 * o, b + 4 * o, c + 4 * o, s, o);
        //        step<1, S, T>::apply(a + 5 * o, b + 5 * o, c + 5 * o, s, o);
        //        step<1, S, T>::apply(a + 6 * o, b + 6 * o, c + 6 * o, s, o);
        //        step<1, S, T>::apply(a + 7 * o, b + 7 * o, c + 7 * o, s, o);
        //        step<1, S, T>::apply(a + 8 * o, b + 8 * o, c + 8 * o, s, o);
        //        step<1, S, T>::apply(a + 9 * o, b + 9 * o, c + 9 * o, s, o);
        //    }
        //};

        /// <summary>
        /// Template specialisation which actually performs the
        /// <see cref="trrojan::stream::task_type::add" /> task.
        /// </summary>
        template<scalar_type S> struct step<1, S, task_type::add> {
            typedef typename scalar_type_traits<S>::type scalar_type;

            static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
                    const scalar_type *b, scalar_type *c, const scalar_type s,
                    const size_t o) {
                *c = *a + *b;
            }
        };

        /// <summary>
        /// Template specialisation which actually performs the
        /// <see cref="trrojan::stream::task_type::copy" /> task.
        /// </summary>
        template<scalar_type S> struct step<1, S, task_type::copy> {
            typedef typename scalar_type_traits<S>::type scalar_type;

            static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
                    const scalar_type *b, scalar_type *c, const scalar_type s,
                    const size_t o) {
                *c = *a;
            }
        };

        /// <summary>
        /// Template specialisation which actually performs the
        /// <see cref="trrojan::stream::task_type::scale" /> task.
        /// </summary>
        template<scalar_type S> struct step<1, S, task_type::scale> {
            typedef typename scalar_type_traits<S>::type scalar_type;

            static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
                    const scalar_type *b, scalar_type *c, const scalar_type s,
                    const size_t o) {
                *c = s * *a;
            }
        };

        /// <summary>
        /// Template specialisation which actually performs the
        /// <see cref="trrojan::stream::task_type::triad" /> task.
        /// </summary>
        template<scalar_type S> struct step<1, S, task_type::triad> {
            typedef typename scalar_type_traits<S>::type scalar_type;

            static TRROJANSTREAM_FORCE_INLINE void apply(const scalar_type *a,
                    const scalar_type *b, scalar_type *c, const scalar_type s,
                    const size_t o) {
                *c = s * *a + *b;
            }
        };

        /// <summary>
        /// The thread function which invokes the
        /// <see cref="trrojan::stream::worker_thread::dispatch" />
        /// cascade to to run the actual tests.
        /// </summary>
        /// <remarks>
        /// <para>This method is only responsible for switching from
        /// <c>static</c> scope to the scope of <paramref name="param" /> and
        /// then invoking the begin of the cascade of calls to
        /// <see cref="trrojan::stream::worker_thread::dispatch" />. The actual
        /// work is performed in the last step of this cascade.</para>
        /// <para>The method also ensures that all required locks are held while
        /// the dispatch cascade is executed.</para>
        /// </remarks>
        /// <param name="param">A pointer to the
        /// <see cref="trrojan::stream::worker_thread" /> object that holds the
        /// handle and the configuration.</param>
        /// <returns>0</returns>
#ifdef _WIN32
        static DWORD WINAPI thunk(void *param);
#else /* _WIN32 */
        static void *thunk(void *param);
#endif /* _WIN32 */


        /// <summary>
        /// Selects the specified scalar type <paramref name="s" /> for
        /// execution and continues with dispatching the problem size.
        /// </summary>
        template<trrojan::stream::scalar_type S,
            trrojan::stream::scalar_type... Ss>
        inline void dispatch(
                trrojan::stream::scalar_type_list_t<S, Ss...>,
                const trrojan::stream::scalar_type s,
                const trrojan::stream::access_pattern a,
                const problem_size_type p,
                const trrojan::stream::task_type t) {
            if (S == s) {
                //std::cout << "scalar type " << (int) S << " selected." << std::endl;
                this->dispatch<S>(problem_sizes(), a, p, t);
            } else {
                this->dispatch(
                    trrojan::stream::scalar_type_list_t<Ss...>(),
                    s, a, p, t);
            }
        }

        /// <summary>
        /// Recursion stop.
        /// </summary>
        inline void dispatch(
            trrojan::stream::scalar_type_list_t<>,
            const trrojan::stream::scalar_type s,
            const trrojan::stream::access_pattern a,
            const problem_size_type p,
            const trrojan::stream::task_type t) { }

        /// <summary>
        /// Selects the specified problem size <paramref name="p" /> for
        /// execution and continues with dispatching the access pattern.
        /// </summary>
        template<trrojan::stream::scalar_type S,
            problem_size_type P, problem_size_type... Ps>
        inline void dispatch(
                trrojan::integer_sequence<problem_size_type, P, Ps...>,
                const trrojan::stream::access_pattern a,
                const problem_size_type p,
                const trrojan::stream::task_type t) {
            if (P == p) {
                //std::cout << "problem size " << P << " (" << p << ") selected." << std::endl;
                this->dispatch<S, P>(access_pattern_list(), a, t);
            } else {
                this->dispatch<S>(
                    trrojan::integer_sequence<problem_size_type, Ps...>(),
                    a, p, t);
            }
        }

        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<trrojan::stream::scalar_type S>
        inline void dispatch(
            trrojan::integer_sequence<problem_size_type>,
            const trrojan::stream::access_pattern a,
            const problem_size_type p,
            const trrojan::stream::task_type t) { }

        /// <summary>
        /// Selects the specified access pattern <paramref name="a" /> for
        /// execution.
        /// </summary>
        template<trrojan::stream::scalar_type S, problem_size_type P,
            trrojan::stream::access_pattern A,
            trrojan::stream::access_pattern... As>
        inline void dispatch(trrojan::stream::access_pattern_list_t<A, As...>,
                const trrojan::stream::access_pattern a,
                const trrojan::stream::task_type t) {
            if (A == a) {
                //std::cout << "access pattern " << (int) A << " selected." << std::endl;
                this->dispatch<S, P, A>(task_type_list(), t);
            } else {
                this->dispatch<S, P>(
                    trrojan::stream::access_pattern_list_t<As...>(),
                    a, t);
            }
        }

        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<trrojan::stream::scalar_type S, problem_size_type P>
        inline void dispatch(trrojan::stream::access_pattern_list_t<>,
            const trrojan::stream::access_pattern a,
            const trrojan::stream::task_type t) { }


        /// <summary>
        /// Selects the specified task type <paramref name="t" /> for
        /// execution.
        /// </summary>
        template<trrojan::stream::scalar_type S,
            trrojan::stream::worker_thread::problem_size_type P,
            trrojan::stream::access_pattern A,
            trrojan::stream::task_type T,
            trrojan::stream::task_type... Ts>
        void dispatch(trrojan::stream::task_type_list_t<T, Ts...>,
                const trrojan::stream::task_type t);

        /// <summary>
        /// Recursion stop.
        /// </summary>
        template<trrojan::stream::scalar_type S,
            trrojan::stream::worker_thread::problem_size_type P,
            trrojan::stream::access_pattern A>
        inline void dispatch(trrojan::stream::task_type_list_t<>,
            const trrojan::stream::task_type t) { }

        /// <summary>
        /// Synchronises the worker threads using the same
        /// <see cref="trrojan::stream::worker_thread::barrier" />
        /// for the <see cref="barrierId" />th iteration using busy
        /// waiting (spin lock).
        /// </summary>
        void synchronise(const int barrierId);

        /// <summary>
        /// The barrier synchronising the test.
        /// </summary>
        barrier_type barrier;

        /// <summary>
        /// The native thread handle.
        /// </summary>
        handle_type hThread;

        /// <summary>
        /// Stores the problem, which is shared between the threads.
        /// </summary>
        problem_type _problem;

        /// <summary>
        /// The rank of the thread, which determines the offset
        /// </summary>
        rank_type rank;

        /// <summary>
        /// The results of the thread.
        /// </summary>
        results_type results;

        /// <summary>
        /// The lock for <see cref="results" />.
        /// </summary>
        mutable std::mutex results_lock;
    };

}
}

#include "trrojan/stream/worker_thread.inl"
