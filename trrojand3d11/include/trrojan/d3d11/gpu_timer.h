// <copyright file="gpu_timer.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

/*
 * include\the\graphics\directx\d3d11_performance_counter.h
 *
 * Copyright (C) 2015 TheLib Team (http://www.thelib.org/license)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of TheLib, TheLib Team, nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THELIB TEAM AS IS AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THELIB TEAM BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <vector>

#include <Windows.h>
#include <d3d11.h>

#include <winrt/base.h>

#include "trrojan/constants.h"
#include "trrojan/timer.h"

#include "trrojan/d3d11/device.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// A performance counter using D3D11 timestamp queries.
    /// </summary>
    /// <remarks>
    /// <para>At most one counter should be created for each device. If multiple
    /// timings should be taken in each frame, allocate sufficient timestamp
    /// queries in this single counter.</para>
    /// <para>The performance counter is <tparamref name="L" />-buffered, ie
    /// there are <tparamref name="L" /> complete sets of counters. One is 
    /// active and used for issuing queries, the one before that is used for
    /// evaluating the queries. This ensures that reading the queries does not
    /// stall the CPU.</para>
    /// <para>You can also set the number of buffers to 1 and use the
    /// <c>try_</c>... methods to check for yourself whether you can read the
    /// buffers. However, you cannot render the next frame in this case.</para>
    /// <para>The implementation follows the description on
    /// http://www.reedbeta.com/blog/2011/10/12/gpu-profiling-101/ and is
    /// heavily borrowed from THElib
    /// (https://sourceforge.net/p/thelib/thesvn/HEAD/tree/trunk/thelib++/include/the/graphics/directx/d3d11_performance_counter.h).
    /// </para>
    /// <para>A typical way of using the counter would be:
    /// <code>
    /// the::graphics::directx::d3d11_performance_counter<2> pc;
    /// pc.initialise(device, 2);
    /// 
    /// foreach frame {
    ///     pc.start_frame();
    ///     pc.start(0);
    ///     // Do something.
    ///     pc.start(1);
    ///     // Do something you want to measure separately.
    ///     pc.end(1);
    ///     // Continue doing something.
    ///     pc.end(0)
    ///     pc.end_frame();
    ///     if (pc.can_evaluate()) {
    ///         bool isDisjoint = false;
    ///         UINT64 frequency = 0;
    ///         INT64 difference = 0;
    ///         pc.evaluate_frame(isDisjoint, frequency);
    ///         if (!isDisjoint) {
    ///             difference = pc.evaluate(0);
    ///             difference = pc.evaluate(1);
    ///         }
    ///     }
    /// }
    /// </code>
    /// </para>
    /// <para>Alternatively, you can replace <c>pc.end_frame()</c> and everything
    /// below with:
    /// <code>
    /// std::vector<d3d11_performance_counter::fractional_value_type> diffs;
    /// pc.end_frame(std::back_inserter(diffs));
    /// <code>
    /// to end the frame and to retrieve all available data in milliseconds at
    /// the same time.This method might, however, not return anything in case
    /// insufficient data are available or the timer is disjoint.</para>
    /// </remarks>
    /// <tparam name="L">The latency when reading the results. This should be
    /// two or more in order to double buffer the evaluation of the resuls.
    /// </tparam>
    template<size_t L = 1> class gpu_timer {

        static_assert(L >= 1, "There must be at least one slot for counters, "
            "better would be two or more.");

    public:

        /// <summary>
        /// Represents the difference between two points in time.
        /// </summary>
        typedef trrojan::timer::difference_type difference_type;

        /// <summary>
        /// The type to represent milliseconds.
        /// </summary>
        typedef trrojan::timer::millis_type millis_type;

        /// <summary>
        /// The type that is used to count the number of available queries.
        /// </summary>
        typedef size_t size_type;

        /// <summary>
        /// Represents a point in time.
        /// </summary>
        typedef trrojan::timer::value_type value_type;

        /**
        * Convert a full resolution performance counter difference to
        * milliseconds assuming the given frequency.
        *
        * @param value The full resolution value to be converted.
        *
        * @return The milliseconds that 'value' represents.
        */
        static millis_type to_milliseconds(
            const difference_type value, const value_type frequency);

        /**
        * Convert a full resolution performance counter value to
        * milliseconds assuming the given frequency.
        *
        * @param value The full resolution value to be converted.
        *
        * @return The milliseconds that 'value' represents.
        */
        static millis_type to_milliseconds(const value_type value,
            const value_type frequency);

        /// <summary>
        /// Indicates an infinite number of retries.
        /// </summary>
        static const size_type infinite;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <remarks>
        /// The performance counter cannot yet be used, but must be associated
        /// with a specific D3D device using <see cref="initialise" />.
        /// </remarks>
        inline gpu_timer(void) : idxActiveQuery(0) { }

        gpu_timer(const gpu_timer&) = delete;

        gpu_timer(gpu_timer&&) = delete;

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        virtual ~gpu_timer(void);

        /**
        * Answer whether enough frames have been measured to evaluate one.
        *
        * @return true if it is possible to evaluate a frame, false otherwise.
        */
        inline bool can_evaluate(void) const {
            auto that = const_cast<gpu_timer *>(this);
            return (that->passive_query().disjoint_query != nullptr);
        }

        /**
        * Ends a frame by finalising the timestamp disjoint query.
        *
        * The performance counter must have been initialised before this method
        * can be called. The method does, however, not check this!
        */
        void end_frame(void);

        /**
        * Convenience method that ends a frame and retrieves the values of
        * all counters (in milliseconds) as far as possible.
        *
        * All preconditions of end_frame(), evaluate_frame() and
        * evaluate() must be fulfiled as the method combines there for
        * ease of use.
        *
        * This is the most convenient way of ending a frame and retrieving
        * everything that was measured.
        *
        * @param oit       An output iterator for writing the counter values.
        * @param isLenient If true, the method will not fail if a counter value
        *                  could not be retrieved (eg because it was not
        *                  measured during the last frame). The value will be
        *                  negative in this case.
        *
        * @return true if values have been retrieve, false otherwise, ie if
        *         not enough frames have been measured before.
        *
        * @throws the::system::com_exception In case the operation failed.
        */
        template<class C>
        typename std::enable_if<std::is_same<
            typename C::value_type,
            typename trrojan::d3d11::gpu_timer<L>::millis_type>::value,
            bool>::type
            end_frame(std::back_insert_iterator<C> oit,
                const bool isLenient = false);

        /**
        * End the 'query'th performance query.
        *
        * start() must have been called before for the same query in the same
        * frame. The method does, however, not check this, nor does it perform
        * a range check of 'query'!
        *
        * @param query The index of the query, which must be within
        *              [0, this->size()[.
        */
        void end(const size_type query);

        /**
        * Evaluates the 'query'th timestamp query.
        *
        * You should only call this method after you have evaluated the frame
        * and verified that the GPU counter is not disjoint using
        * evaluate_frame() or try_evaluate_frame().
        *
        * The performance counter must have been initialised before this method
        * can be called and can_evaluate() must yield true. The method does,
        * however, not check this, not does it perform a range check of
        * 'query'!
        *
        * @param outStart Receives the value of the 'query'th start query.
        * @param outEnd   Receives the value of the 'query'th end query.
        * @param query    The number of the query to be evaluated. This must be
        *                 within [0, this->size()[.
        *
        * @throws the::system::com_exception In case the operation failed and
        *                                    cannot be retried.
        * @throws the::invalid_operation_exception If can_evaluate() yields
        *                                          false. The query can never
        *                                          succeed in this case.
        */
        void evaluate(value_type& outStart, value_type& outEnd,
            const size_type query);

        /**
        * Evaluates the 'query'th timestamp query.
        *
        * You should only call this method after you have evaluated the frame
        * and verified that the GPU counter is not disjoint using
        * evaluate_frame() or try_evaluate_frame().
        *
        * The performance counter must have been initialised before this method
        * can be called and can_evaluate() must yield true. The method does,
        * however, not check this, not does it perform a range check of
        * 'query'!
        *
        * @param query The number of the query to be evaluated. This must be
        *              within [0, this->size()[.
        *
        * @return The difference between the end and the start of the 'query'th
        *         query.
        *
        * @throws the::system::com_exception In case the operation failed and
        *                                    cannot be retried.
        * @throws the::invalid_operation_exception If can_evaluate() yields
        *                                          false. The query can never
        *                                          succeed in this case.
        */
        difference_type evaluate(const size_type query);

        /**
        * Evaluates the timestamp distjoint query thus determining  whether
        * the timer values are valid.
        *
        * The performance counter must have been initialised before this method
        * can be called. The method does, however, not check this!
        *
        * @param outIsDisjoint true if the counter has become disjoint, ie if
        *                      the timestamp valued cannot be used.
        * @param outFrequency  The frequency of the GPU counter in Hz.
        *
        * @throws the::system::com_exception In case the operation failed and
        *                                    cannot be retried.
        * @throws the::invalid_operation_exception If can_evaluate() yields
        *                                          false. The query can never
        *                                          succeed in this case.
        */
        void evaluate_frame(bool& outIsDisjoint, value_type& outFrequency,
            value_type timeout = gpu_timer::infinite);

        /**
        * Associates the performance counter with the specified device and
        * prepares 'cntPerFrameQueries' for being issued between the calls
        * to start_frame() and end_frame().
        *
        * @param device
        * @param cntPerFrameQueries
        *
        * @throws the::argument_null_exception If 'device' is nullptr.
        * @throws the::invalid_operation_exception If the object has already
        *                                          been initialised.
        * @throws the::system::com_exception If one or more queries could not
        *                                    be created.
        */
        void initialise(ID3D11Device *device,
            const size_type cntPerFrameQueries = 1);

        /// <summary>
        /// Associates the performance counter with the specified device and
        /// prepares <paramref name="cntPerFrameQueries" /> for being issued
        /// between the calls to <see cref="start_frame" /> and
        /// <see cref="end_frame" />.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="cntPerFrameQueries"></param>
        inline void initialise(winrt::com_ptr<ID3D11Device> device,
                const size_type cntPerFrameQueries = 1) {
            this->initialise(device.get(), cntPerFrameQueries);
        }

        /**
        * Resizes the number of queries that can be issued per frame.
        *
        * This should only performed if there are not outstanding queries, ie
        * not between start_frame() and end_frame().
        *
        * Resizing the number of buffers invalidates all possibly collected
        * data.
        *
        * @param cntPerFrameQueries
        *
        * @throws the::system::com_exception If one or more queries could not
        *                                    be created.
        */
        void resize(const size_type cntPerFrameQueries);

        /**
        * Answer the number of queries that can be issued for each frame.
        *
        * @return The number of queries that can the performance counter has.
        */
        inline size_type size(void) const {
            auto that = const_cast<d3d11_performance_counter *>(this);
            assert(that->active_query().timestamp_end_query.size()
                == that->active_query().timestamp_start_query.size());
            assert(that->active_query().timestamp_end_query.size()
                == that->passive_query().timestamp_end_query.size());
            return that->active_query().timestamp_end_query.size();
        }

        /**
        * Starts a new frame by issuing a timestamp disjoint query into the
        * command stream.
        *
        * The performance counter must have been initialised before this method
        * can be called. The method does, however, not check this!
        */
        void start_frame(void);

        /**
        * Start the 'query'th performance query.
        *
        * start_frame() must have been called before. The method does, however,
        * not check this, nor does it perform a range check of 'query'!
        *
        * @param query The index of the query, which must be within
        *              [0, this->size()[.
        */
        void start(const size_type query);

        /**
        * Tries to evaluate the 'query'th timestamp query.
        *
        * You should only call this method after you have evaluated the frame
        * and verified that the GPU counter is not disjoint.
        *
        * The performance counter must have been initialised before this method
        * can be called. The method does, however, not check this, not does
        * it perform a range check of 'query'!
        *
        * @param outStart Receives the value of the 'query'th start query.
        * @param outEnd   Receives the value of the 'query'th end query.
        * @param query    The number of the query to be evaluated. This must be
        *                 within [0, this->size()[.
        *
        * @return true if both queries have been successfully retrieved and
        *         stored in 'outStart' and 'outEnd',
        *         false if the method should be called again later. In case
        *         can_evaluate() returns false, measure more frames before
        *         calling the method again.
        *
        * @throws the::system::com_exception In case the operation failed and
        *                                    cannot be retried.
        */
        bool try_evaluate(value_type& outStart, value_type& outEnd,
            const size_type query);

        /**
        * Tries to evaluate the timestamp distjoint query thus determining
        * whether the timer values are valid.
        *
        * The performance counter must have been initialised before this method
        * can be called. The method does, however, not check this!
        *
        * @param outIsDisjoint true if the counter has become disjoint, ie if
        *                      the timestamp valued cannot be used.
        * @param outFrequency  The frequency of the GPU counter in Hz.
        *
        * @return true if the data have been retrieved and stored to
        *         'outIsDisjoint' and 'outFrequency',
        *         false if the method should be called again later. In case
        *         can_evaluate() returns false, measure more frames before
        *         calling the method again.
        *
        * @throws the::system::com_exception In case the operation failed and
        *                                    cannot be retried.
        */
        bool try_evaluate_frame(bool& outIsDisjoint, value_type& outFrequency);

        gpu_timer& operator =(const gpu_timer&) = delete;

        gpu_timer& operator =(gpu_timer&&) = delete;

    private:

        typedef winrt::com_ptr<ID3D11DeviceContext> context_type;
        typedef winrt::com_ptr<ID3D11Device> device_type;
        typedef winrt::com_ptr<ID3D11Query> query_type;
        typedef std::vector<query_type> query_list_type;

        /// <summary>
        /// Groups the data which are used for latency buffering.
        /// </summary>
        struct query_data {

            /// <summary>
            /// Answer whether a timestamp query returns reliable values and 
            /// provides the frequency of the performance counter.
            /// </summary>
            /// <remarks>
            /// The timestamp disjoint query encloses the whole frame, ie there
            /// is only one per device.
            /// </remarks>
            query_type disjoint_query;

            /// <summary>
            /// The timestamp query issued at the end of the measurement.
            /// </summary>
            query_list_type timestamp_end_query;

            /// <summary>
            /// The timestamp query issued at the begin of the measurement.
            /// </summary>
            query_list_type timestamp_start_query;
        };

        /// <summary>
        /// Answer the active query block (the one, we are using to measure the
        /// current frame).
        /// </summary>
        inline query_data& active_query(void) {
            return this->queries[this->idxActiveQuery];
        }

        /// <summary>
        /// Ensures that <paramref name="size" /> timestamp queries are in
        /// <paramref name="list" />.
        /// </summary>
        /// <param name="list">The list to be modified.</param>
        /// <param name="size">The requested number of queries.</param>
        /// <exception cref="std::system_error">If one or more queries could
        /// not be created.</exception>
        void assert_queries(query_list_type& list, const size_type size);

        /// <summary>
        /// Answer the passive query block (the one, we are using to evaluate the
        /// queries).
        /// </summary>
        inline query_data& passive_query(void) {
            size_t idxPassiveQuery = (this->idxActiveQuery + 1) % L;
            return this->queries[idxPassiveQuery];
        }

        /// <summary>
        /// The device that we create the queries on.
        /// </summary>
        device_type device;

        /// <summary>
        /// The index of the queries that are used for the current frame.
        /// </summary>
        size_t idxActiveQuery;

        /// <summary>
        /// The immediate context of <see cref="device" />.
        /// </summary>
        context_type immediateContext;

        /// <summary>
        /// The buffer for the queries.
        /// </summary>
        std::array<query_data, L> queries;
    };

}
}

#include "trrojan/d3d11/gpu_timer.inl"
