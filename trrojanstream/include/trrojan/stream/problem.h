/// <copyright file="problem.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <vector>

#include "trrojan/constants.h"
#include "trrojan/timer.h"
#include "trrojan/variant.h"

#include "trrojan/stream/access_pattern.h"
#include "trrojan/stream/export.h"
#include "trrojan/stream/scalar_type.h"
#include "trrojan/stream/task_type.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// Provides all storage for a stream benchmark problem.
    /// </summary>
    class TRROJANSTREAM_API problem {

    public:

        typedef trrojan::stream::access_pattern access_pattern_t;
        typedef std::shared_ptr<problem> pointer_type;
        typedef trrojan::stream::scalar_type scalar_type_t;
        typedef trrojan::stream::task_type task_type_t;

        /// <summary>
        /// The default value for the number of iterations.
        /// </summary>
        static const size_t default_iterations = 10;

        /// <summary>
        /// The default value for the problem size.
        /// </summary>
        static const size_t default_problem_size = 2000000;

        /// <summary>
        /// Creates a new problem with the specified properties.
        /// </summary>
        problem(const scalar_type_t scalar,
            const trrojan::variant& value,
            const task_type_t task,
            const access_pattern_t pattern,
            const size_t size = default_problem_size,
            const size_t iterations = default_iterations,
            const size_t parallelism = 1);

        /// <summary>
        /// Gets the first input array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *a(void) {
            return static_cast<T *>(static_cast<void *>(this->_a.data()));
        }

        /// <summary>
        /// Gets the first input array.
        /// </summary>
        /// <returns></returns>
        template<scalar_type_t T>
        inline typename scalar_type_traits<T>::type *a(void) {
            return this->a<typename scalar_type_traits<T>::type>();
        }

        /// <summary>
        /// Answer the access pattern to be used when processing the problem.
        /// </summary>
        inline trrojan::stream::access_pattern access_pattern(void) const {
            return this->_access_pattern;
        }

        /// <summary>
        /// Gets the second input array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *b(void) {
            return static_cast<T *>(static_cast<void *>(this->_b.data()));
        }

        /// <summary>
        /// Gets the second input array.
        /// </summary>
        /// <returns></returns>
        template<scalar_type_t T>
        inline typename scalar_type_traits<T>::type *b(void) {
            return this->b<typename scalar_type_traits<T>::type>();
        }

        /// <summary>
        /// Gets the output array.
        /// </summary>
        /// <returns></returns>
        template<class T> inline T *c(void) {
            return static_cast<T *>(static_cast<void *>(this->_c.data()));
        }

        /// <summary>
        /// Gets the output array.
        /// </summary>
        /// <returns></returns>
        template<scalar_type_t T>
        inline typename scalar_type_traits<T>::type *c(void) {
            return this->c<typename scalar_type_traits<T>::type>();
        }

        /// <summary>
        /// Given the size of this problem, convert the given runtime of a
        /// thread to MB/s under the assumption that the given number of memory
        /// accesses have been performed for each item.
        /// </summary>
        /// <param name="dt">The time one thread took to complete the benchmark
        /// (in milliseconds).</param>
        /// <param name="cnt_accesses">The number of memory accesses (reads and
        /// writes) per item.</param>
        /// <returns>The transfer rate in MB/s.</returns>
        inline double calc_thread_mb_per_s(const timer::millis_type dt,
                const size_t cnt_accesses) const {
            typedef trrojan::constants<double> constants;
            auto s = dt / constants::millis_per_second;
            auto m = static_cast<double>(this->size_in_bytes());
            m /= constants::bytes_per_megabyte;
            return (m / s * cnt_accesses);
        }

        /// <summary>
        /// Given the size of this problem, convert the given total runtime to
        /// MB/s under the assumption that the given number of memory acceses
        /// have been performed for each item.
        /// </summary>
        /// <param name="dt">The time it took to complete the benchmark (in
        /// milliseconds).</param>
        /// <param name="cnt_accesses">The number of memory accesses (reads and
        /// writes) per item.</param>
        /// <returns>The transfer rate in MB/s.</returns>
        inline double calc_total_mb_per_s(const timer::millis_type dt,
                const size_t cnt_accesses) const {
            typedef trrojan::constants<double> constants;
            auto s = dt / constants::millis_per_second;
            auto m = static_cast<double>(this->total_size_in_bytes());
            m /= constants::bytes_per_megabyte;
            return (m / s * cnt_accesses);
        }

        /// <summary>
        /// Answer the number of iterations to perform for the same problem.
        /// </summary>
        inline size_t iterations(void) const {
            return this->_iterations;
        }

        /// <summary>
        /// Answer for how many threads the problem is intended.
        /// </summary>
        /// <returns></returns>
        inline size_t parallelism(void) const {
            return this->_parallelism;
        }

        /// <summary>
        /// Gets the scalar value
        /// </summary>
        /// <returns></returns>
        template<class T> inline T s(void) {
            return this->_scalar_value.as<T>();
        }

        /// <summary>
        /// Gets the scalar value.
        /// </summary>
        /// <returns></returns>
        template<scalar_type_t T>
        inline typename scalar_type_traits<T>::type s(void) {
            return this->s<typename scalar_type_traits<T>::type>();
        }

        /// <summary>
        /// Answers the size of a single scalar.
        /// </summary>
        inline size_t scalar_size(void) const {
            return this->_scalar_size;
        }

        /// <summary>
        /// Answer the <see cref="trrojan::stream::scalar_type" /> which for the
        /// problem has been initialised.
        /// </summary>
        /// <returns></returns>
        inline scalar_type_t scalar_type(void) const {
            return this->_scalar_type;
        }

        /// <summary>
        /// Answer the problem size (in number of elements) for a single thread.
        /// </summary>
        /// <returns></returns>
        inline size_t size(void) const {
            auto s = this->total_size_in_bytes();
            auto sp = (this->_scalar_size * this->_parallelism);
            assert(s % sp == 0);
            return (s / sp);
        }

        /// <summary>
        /// Answer the problem size (in bytes) for a single thread.
        /// </summary>
        inline size_t size_in_bytes(void) const {
            assert(this->_a.size() == this->_b.size());
            assert(this->_a.size() == this->_c.size());
            assert(this->_a.size() % this->_parallelism == 0);
            return (this->_a.size() / this->_parallelism);
        }

        /// <summary>
        /// Answer the task to be performed on the memory.
        /// </summary>
        inline task_type_t task_type(void) const {
            return this->_task_type;
        }

        /// <summary>
        /// Answer the combined problem size for all threads in number of
        /// elements.
        /// </summary>
        inline size_t total_size(void) const {
            auto s = this->total_size_in_bytes();
            assert(s % this->_scalar_size == 0);
            return (s / this->_scalar_size);
        }

        /// <summary>
        /// Answer the combined problem size for all threads in bytes.
        /// </summary>
        inline size_t total_size_in_bytes(void) const {
            assert(this->_a.size() == this->_b.size());
            assert(this->_a.size() == this->_c.size());
            return this->_a.size();
        }

    private:

        typedef std::vector<std::uint8_t> problem_type;

        /// <summary>
        /// Allocates <see cref="trrojan::stream::problem::_a" />,
        /// <see cref="trrojan::stream::problem::_b" /> and
        /// <see cref="trrojan::stream::problem::_c" /> to hold
        /// <paramref name="cnt" /> elements of type <tparamref name="T" />.
        /// </summary>
        template<scalar_type_t T> void allocate(size_t cnt);

        /// <summary>
        /// The first input array.
        /// </summary>
        problem_type _a;

        /// <summary>
        /// The access pattern to be used when processing the problem.
        /// </summary>
        access_pattern_t _access_pattern;

        /// <summary>
        /// The second input array.
        /// </summary>
        problem_type _b;

        /// <summary>
        /// The output array.
        /// </summary>
        problem_type _c;

        /// <summary>
        /// The number of iterations to perform for the same problem.
        /// </summary>
        size_t _iterations;

        /// <summary>
        /// The number of threads the problem is for.
        /// </summary>
        size_t _parallelism;

        /// <summary>
        /// Remembers the size of a single scalar.
        /// </summary>
        size_t _scalar_size;

        /// <summary>
        /// The type of a scalar.
        /// </summary>
        scalar_type_t _scalar_type;

        /// <summary>
        /// The value of the scalar.
        /// </summary>
        trrojan::variant _scalar_value;

        /// <summary>
        /// The task to be performed on the memory.
        /// </summary>
        task_type_t _task_type;
    };

}
}

#include "trrojan/stream/problem.inl"
