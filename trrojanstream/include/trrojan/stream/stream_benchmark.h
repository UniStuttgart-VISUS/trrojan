/// <copyright file="stream_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/benchmark.h"

#include <atomic>
#include <memory>

#include "trrojan/stream/export.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// The implementation of the memory streaming benchmark.
    /// </summary>
    class TRROJANSTREAM_API stream_benchmark : public trrojan::benchmark_base {

    public:

        /// <summary>
        /// This access pattern instructs each thread to access a continguous
        /// range of memory, ie the threads read from a potentially widespread
        /// memory range at the same time.
        /// </summary>
        static const std::string access_pattern_contiguous;

        /// <summary>
        /// This access pattern instruct the threads to access the memory in an
        /// interleaved manner, ie all threads read in a spatially close range
        /// at approximately the same time.
        /// </summary>
        static const std::string access_pattern_interleaved;

        inline stream_benchmark(void) : trrojan::benchmark_base("stream") { }

        virtual ~stream_benchmark(void);

        virtual result_set run(const configuration_set& configs);

    private:

        /// <summary>
        /// A spin lock used as a barrier to ensure simultaneous memory access.
        /// </summary>
        typedef std::shared_ptr<std::atomic<int>> barrier_type;


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

        //template <int DIM, typename T>
        //class DotProduct {
        //public:
        //    static T result(T* a, T* b) {
        //        return *a * *b + DotProduct<DIM - 1, T>::result(a + 1, b + 1);
        //    }
        //};

    };

}
}
