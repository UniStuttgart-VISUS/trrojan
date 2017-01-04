/// <copyright file="worker_thread.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"

#include <array>
#include <atomic>
#include <cinttypes>
#include <climits>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else /* _WIN32 */
#include <pthread.h>
#endif /* _WIN32 */

#include "trrojan/constants.h"
#include "trrojan/log.h"
#include "trrojan/timer.h"

#include "trrojan/stream/export.h"
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
    class TRROJANSTREAM_API worker_thread {

    public:

        /// <summary>
        /// A spin lock used as a barrier to ensure simultaneous memory access.
        /// </summary>
        typedef std::shared_ptr<std::atomic<int>> barrier_type;

        /// <summary>
        /// A pointer to a <see cref="worker_thread" />.
        /// </summary>
        typedef std::shared_ptr<worker_thread> pointer_type;

        /// <summary>
        /// A type to express a thread's rank.
        /// </summary>
        typedef std::size_t rank_type;

        static pointer_type create(const rank_type rank,
            const uint64_t affinity_mask = 0,
            const uint16_t affinity_group = 0);

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

        void start(const rank_type rank, const uint64_t affinity_mask = 0,
            const uint16_t affinity_group = 0);

    private:

        /// <summary>
        /// The thread function that performs the actual work.
        /// </summary>
        /// <param name="param">A pointer to the thread object that holds the
        /// handle and the configuration.</param>
        /// <returns>0</returns>
#ifdef _WIN32
        static DWORD WINAPI thunk(void *param);
#else /* _WIN32 */
        static void *thunk(void *param);
#endif /* _WIN32 */

        worker_thread(const worker_thread&) = delete;

        worker_thread& operator =(const worker_thread&) = delete;

        /// <summary>
        /// The type of a native thread handle.
        /// </summary>
#ifdef _WIN32
        typedef HANDLE handle_type;
#else /* _WIN32 */
        typedef pthread_t handle_type;
#endif /* _WIN32 */

        /// <summary>
        /// The barrier synchronising the test.
        /// </summary>
        barrier_type barrier;

        /// <summary>
        /// The native thread handle.
        /// </summary>
        handle_type hThread;

        /// <summary>
        /// The rank of the thread, which determines the offset
        /// </summary>
        rank_type rank;


        /// <summary>
        /// Gets the end of the iteration for the thread with the given rank
        /// considering the configured access pattern.
        /// </summary>
        /// <remarks>
        /// <para>The returned value is the array index <i>after</i> the last
        /// element to be read, ie it can be used like the array size in a for
        /// loop.</para>
        /// <para>In case of a contiguous access pattern, the last thread is
        /// assumed to perform more work in case the problem size is not
        /// divisble by the number of worker threads.</para>
        /// </remarks>
        /*inline size_type end(const size_type rank) const {
            auto isLast = (rank == (this->number_of_threads() - 1));
            auto p = this->problem_size();
            auto t = this->number_of_threads();
            auto r = p % t;
            return (this->access_pattern() == access_pattern::contiguous)
                ? (isLast ? p : this->offset(rank + 1))
                : (p - r - t + rank + 1 + ((rank < r) ? t : 0));
        }*/

        /// <summary>
        /// Gets the start offset of the thread with the given rank considering the
        /// configured access pattern.
        /// </summary>
        /*inline size_type offset(const size_type rank) const {
            assert(this->problem_size() > this->number_of_threads());
            return (this->access_pattern() == access_pattern::contiguous)
                ? rank * this->problem_size() / this->number_of_threads()
                : rank;
        }*/

        /// <summary>
        /// Answer how big the share of the <paramref name="rank" />th thread is.
        /// </summary>
        /*inline size_type problem_size(const size_type rank) const {
            auto p = this->problem_size();
            auto t = this->number_of_threads();
            return (this->access_pattern() == access_pattern::contiguous)
                ? (this->end(rank) - this->offset(rank))
                : ((p / t) + ((rank < (p % t)) ? 1 : 0));
        }*/

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

    public:
        inline static void crowbar() {
            typedef step<1000000, scalar_type::float32, task_type::copy> step_type;
            std::vector<float> a(1000000);
            std::vector<float> b(1000000);
            std::vector<float> c(1000000);

            std::generate(a.begin(), a.end(), std::rand);
            std::generate(b.begin(), b.end(), std::rand);


            trrojan::timer timer;
            timer.start();
            step_type::apply(a.data(), b.data(), c.data(), 12.4f, 1);
            auto elapsed = timer.elapsed_millis();
            auto gigs = ((double) a.size() * sizeof(float)) / trrojan::bytes_per_gigabyte;
            std::cout << "stream elapsed: " << elapsed << " " << gigs  << " " << gigs * millis_per_second / elapsed << std::endl;
            std::cout << "verify: " << worker_thread::verify(a.data(), b.data(), c.data(), 12.4f, a.size(), task_type::copy) << std::endl;
        }
    };

}
}

#include "trrojan/stream/worker_thread.inl"
