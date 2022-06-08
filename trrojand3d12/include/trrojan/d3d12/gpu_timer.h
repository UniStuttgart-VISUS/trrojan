// <copyright file="gpu_timer.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <atlbase.h>
#include <Windows.h>
#include <d3d12.h>

#include "trrojan/constants.h"
#include "trrojan/timer.h"

#include "trrojan/d3d12/device.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// A performance counter using D3D12 timestamp queries.
    /// </summary>
    /// <remarks>
    /// <para>At most one counter should be created for each device. If multiple
    /// timings should be taken in each frame, allocate sufficient timestamp
    /// queries in this single counter.</para>
    /// <para>A typical way of using the counter would be:
    /// <code>
    /// the::graphics::directx::d3d12_performance_counter pc(device, 2);
    /// 
    /// foreach frame {
    ///     pc.start_frame();
    ///     pc.start(cmd_list, 0);
    ///     // Do something.
    ///     pc.start(cmd_list, 1);
    ///     // Do something you want to measure separately.
    ///     pc.end(cmd_list, 1);
    ///     // Continue doing something.
    ///     pc.end(cmd_list, 0)
    ///     auto frame = pc.end_frame();
    ///     auto outer_difference = pc.evaluate(frame, 0);
    ///     auto inner_difference = pc.evaluate(frame, 1);
    /// }
    /// </code>
    /// </para>
    /// </remarks>
    class gpu_timer final {

    public:

        /// <summary>
        /// Represents the difference between two points in time.
        /// </summary>
        typedef INT64 difference_type;

        /// <summary>
        /// The type to represent milliseconds.
        /// </summary>
        typedef trrojan::timer::millis_type millis_type;

        /// <summary>
        /// The type that is used to count the number of available queries.
        /// </summary>
        typedef UINT size_type;

        /// <summary>
        /// Represents a point in time.
        /// </summary>
        typedef UINT64 value_type;

        /// <summary>
        /// Samples the CPU clock and the GPU timestamp counter of the given
        /// <paramref name="queue" /> at the same time.
        /// </summary>
        /// <remarks>
        /// This method can be used to correlate the result of the CPU
        /// performance counter with GPU timestamps as described on
        /// https://docs.microsoft.com/en-us/windows/win32/direct3d12/timing.
        /// </remarks>
        /// <param name="queue">The direct/compute/copy queue to obtain the
        /// correlated timestamps for.</param>
        /// <param name="gpu_timestamp">Receives the value of the GPU timestamp
        /// counter.</param>
        /// <param name="cpu_timestamp">Receives the value of the CPU
        /// performance counter.</param>
        /// <exception cref="std::argument_exception">If
        /// <paramref name="queue" /> is <c>nullptr</c>.</exception>
        /// <exception cref="ATL::CAtlException">If the operation failed, eg
        /// because the queue does not support timestamp queries.</exception>
        static void get_clock_calibration(ID3D12CommandQueue *queue,
            value_type& gpu_timestamp,
            value_type& cpu_timestamp);

        /// <summary>
        /// Gets the timestamp frequency in Hertz for the given command
        /// <paramref name="queue" />.
        /// </summary>
        /// <param name="queue">The queue to get the frequency for.</param>
        /// <returns>The timestamp frequency in Hertz.</returns>
        /// <exception cref="std::argument_exception">If
        /// <paramref name="queue" /> is <c>nullptr</c>.</exception>
        /// <exception cref="ATL::CAtlException">If the operation failed, eg
        /// because the queue does not support timestamp queries.</exception>
        static value_type get_timestamp_frequency(ID3D12CommandQueue *queue);

        /// <summary>
        /// Convert a full resolution performance counter difference to
        /// milliseconds assuming the given frequency.
        /// </summary>
        /// <param name="value">The full resolution value to be converted.
        /// </param>
        /// <param name="frequency">The timestamp frequency in Hertz (ticks
        /// per second) as returned by 
        /// <see cref="ID3D12CommandQueue::GetTimestampFrequency" />.</param>
        /// <returns>The milliseconds that <paramref name="value" />
        /// represents.</returns>
        static millis_type to_milliseconds(const difference_type value,
            const value_type frequency);

        /// <summary>
        /// Convert a full resolution performance counter value to
        /// milliseconds assuming the given frequency.
        /// </summary>
        /// <param name="value">The full resolution value to be converted.
        /// </param>
        /// <param name="frequency">The timestamp frequency in Hertz (ticks
        /// per second) as returned by 
        /// <see cref="ID3D12CommandQueue::GetTimestampFrequency" />.</param>
        /// <returns>The milliseconds that <paramref name="value" />
        /// represents.</returns>
        static millis_type to_milliseconds(const value_type value,
            const value_type frequency);

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="queries"></param>
        /// <param name="buffers"></param>
        /// <param name="heap"></param>
        gpu_timer(ID3D12Device *device, const size_type queries = 1,
            const size_type buffers = 2,
            const D3D12_QUERY_HEAP_TYPE heap = D3D12_QUERY_HEAP_TYPE_TIMESTAMP);

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        /// <param name="queue"></param>
        /// <param name="queries"></param>
        /// <param name="buffers"></param>
        gpu_timer(ID3D12CommandQueue *queue, const size_type queries = 1,
            const size_type buffers = 2);

        gpu_timer(const gpu_timer&) = delete;

        gpu_timer(gpu_timer&&) = delete;

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~gpu_timer(void);

        /// <summary>
        /// End a frame by resolving the query results of all queries into
        /// the query heap for readback.
        /// </summary>
        /// <remarks>
        /// <para>The query results must be resolved into the buffer in this
        /// timer object before it can be downloaded to the CPU. The recommended
        /// way is downloading everything at once after this method returned.
        /// The data remains available until the next frame is started on the
        /// same slot.</para>
        /// <para>This method does not perform a <c>nullptr</c> check of
        /// <paramref name="cmd_list" /> for performance reasons. Please make
        /// sure to pass only valid command lists.</para>
        /// </remarks>
        /// <param name="cmd_list">The command list to perform the operation
        /// on.</param>
        /// <returns>The frame index that can be used to evaluate (download)
        /// the query results afterwards.</returns>
        size_type end_frame(ID3D12GraphicsCommandList *cmd_list);

        //template<class TContainer>
        //typename std::enable_if<std::is_same<
        //    typename TContainer::value_type, millis_type>::value>::type
        //end_frame(std::back_insert_iterator<TContainer> oit,
        //    ID3D12GraphicsCommandList *cmd_list);

        /// <summary>
        /// End the <paramref name="query" />th performance query by issuing
        /// the timestamp query for the end.
        /// </summary>
        /// <remarks>
        /// <para><see cref="start" /> must have been called before for the same
        /// query in the same frame.The method does, however, not check this.
        /// </para>
        /// <para>This method does not perform a <c>nullptr</c> check of
        /// <paramref name="cmd_list" /> for performance reasons. Please make
        /// sure to pass only valid command lists.</para>
        /// <para> Please make also sure to end the performance query on the
        /// same command list that it has been started, because D3D12 guranatees
        /// that two timestamp queries are not disjoint if and only if they are
        /// in the same command list.</para>
        /// <para>This method does not perform a range check of
        /// <paramref name="query" /> for performance reasons. Make sure to pass
        /// only valid indices.</para>
        /// </remarks>
        /// <param name="cmd_list">The command list in which the timestamp
        /// query is being queued. The same command list should be used for
        /// <see cref="start" /> and <see cref="end" />. This parameter must
        /// not be <c>nullptr</c>.</param>
        /// <param name="query">The index of the query, which must be within
        /// *[0, <see cref="size" />[.</param>
        void end(ID3D12GraphicsCommandList *cmd_list, const size_type query);

        /// <summary>
        /// Evaluates the <paramref name="query" />th timestamp query.
        /// </summary>
        /// <remarks>
        /// <para>This method must only be called after <see cref="end_frame" />
        /// and before <see cref="start_frame" /> was called for the frame index
        /// returned by <see cref="end_frame" /> again (you have the number of
        /// buffers specified as latency for retrieving results). Otherwise,
        /// invalid data will be returned.</para>
        /// <para>It is strongly recommended to use the overload accepting
        /// the output iterator to obtain the results for all queries in a
        /// frame, because this method maps the range if each query individually
        /// wherease the iterator variant maps all results at once.</para>
        /// </remarks>
        /// <param name="outStart">Recevies the result of the timestamp query
        /// at the begin of the measured range.</param>
        /// <param name="outEnd">Receives the result of the timestamp query
        /// at the end of the measured range.</param>
        /// <param name="buffer">The frame/buffer index for which the queries
        /// should be retrieved.</param>
        /// <param name="query">The query index to retrieve, which must be
        /// within [0, <see cref="size" />[.</param>
        /// <exception cref="std::invalid_argument">If either
        /// <paramref name="buffer" /> or <paramref name="query" /> are out of
        /// range.</exception>
        /// <exception cref="ATL::CAtlException">If the buffer holding the
        /// results could not be mapped.</exception>
        void evaluate(value_type& outStart, value_type& outEnd,
            const size_type buffer, const size_type query) const;

        /// <summary>
        /// Evaluates the <paramref name="query" />th timestamp query.
        /// </summary>
        /// <remarks>
        /// <para>This method must only be called after <see cref="end_frame" />
        /// and before <see cref="start_frame" /> was called for the frame index
        /// returned by <see cref="end_frame" /> again (you have the number of
        /// buffers specified as latency for retrieving results). Otherwise,
        /// invalid data will be returned.</para>
        /// <para>It is strongly recommended to use the overload accepting
        /// the output iterator to obtain the results for all queries in a
        /// frame, because this method maps the range if each query individually
        /// wherease the iterator variant maps all results at once.</para>
        /// </remarks>
        /// <param name="buffer">The frame/buffer index for which the queries
        /// should be retrieved.</param>
        /// <param name="query">The query index to retrieve, which must be
        /// within [0, <see cref="size" />[.</param>
        /// <returns>The difference between the end and the begin of the
        /// requested query.</returns>
        /// <exception cref="std::invalid_argument">If either
        /// <paramref name="buffer" /> or <paramref name="query" /> are out of
        /// range.</exception>
        /// <exception cref="ATL::CAtlException">If the buffer holding the
        /// results could not be mapped.</exception>
        difference_type evaluate(const size_type buffer,
            const size_type query) const;

        /// <summary>
        /// Evaluates all queries of the given <paramref name="buffer" />.
        /// </summary>
        /// <typeparam name="TIterator">An output iterator for
        /// <see cref="difference_type" />, which must allow for writing at
        /// least <see cref="size" /> elements.</typeparam>
        /// <param name="oit">The output iterator to receive the differences
        /// between all timestamp queries of the specified frame.</param>
        /// <param name="buffer">The buffer index of the frame for which the
        /// differences should be returned.</param>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="buffer" /> is out of range.</exception>
        /// <exception cref="ATL::CAtlException">If the buffer holding the
        /// results could not be mapped.</exception>
        template<class TIterator>
        void evaluate(TIterator oit, const size_type buffer) const;

        /// <summary>
        /// Resizes the buffer of the timer to support
        /// <paramref name="queries" /> per <paramref name="buffers" />
        /// frame buffers.
        /// </summary>
        /// <param name="queries"></param>
        /// <param name="buffers"></param>
        /// <exception cref="CAtlException">If the result buffer could not be
        /// allocated.</exception>
        void resize(const size_type queries, const size_type buffers = 2);

        /// <summary>
        /// Answer the number of queries that can be issued for each frame.
        /// </summary>
        /// <remarks>
        /// The returned value represents the number of user-define ranges, not
        /// the actual number of queries on the hardware. In practice, each of
        /// the returned number requires two timestamp queries, one at the begin
        /// of the timespan and one at the end.
        /// </remarks>
        /// <returns>The number of queries that can the performance counter has
        /// per frame.</returns>
        inline size_type size(void) const {
            return this->_cnt_queries;
        }

        /// <summary>
        /// Start the <paramref name="query" />th performance query on the given
        /// command list.
        /// </summary>
        /// <remarks>
        /// <para>This method does not perform a <c>nullptr</c> check of
        /// <paramref name="cmd_list" /> for performance reasons. Please make
        /// sure to pass only valid command lists.</para>
        /// <para> Please make also sure to end the performance query on the
        /// same command list that it has been started, because D3D12 guranatees
        /// that two timestamp queries are not disjoint if and only if they are
        /// in the same command list.</para>
        /// <para>This method does not perform a range check of
        /// <paramref name="query" /> for performance reasons. Make sure to pass
        /// only valid indices.</para>
        /// </remarks>
        /// <param name="cmd_list">The command list in which the timestamp
        /// query is being queued. The same command list should be used for
        /// <see cref="start" /> and <see cref="end" />. This parameter must
        /// not be <c>nullptr</c>.</param>
        /// <param name="query">The index of the query, which must be within
        /// *[0, <see cref="size" />.</param>
        void start(ID3D12GraphicsCommandList *cmd_list, const size_type query);

        /// <summary>
        /// Starts a new frame by advancing the buffer index.
        /// </summary>
        void start_frame(void);

        gpu_timer& operator =(const gpu_timer&) = delete;

        gpu_timer& operator =(gpu_timer&&) = delete;

    private:


        /// <summary>
        /// Determines whether a query is a start query or an end query.
        /// </summary>
        enum class query_location : size_type {
            start = 0,
            end = 1
        };

        typedef ATL::CComPtr<ID3D12Device> device_type;
        typedef ATL::CComPtr<ID3D12QueryHeap> heap_type;
        typedef ATL::CComPtr<ID3D12Resource> result_buffer_type;

        /// <summary>
        /// Compute the location of the query result.
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="query"></param>
        /// <param name="location"></param>
        /// <returns></returns>
        inline size_type result_index(const size_type buffer,
                const size_type query, const query_location location) const {
            assert(static_cast<size_type>(location) < 2);
            return buffer * this->queries_per_buffer()
                + 2 * query + static_cast<size_type>(location);
        }

        /// <summary>
        /// Compute the location of the query result.
        /// </summary>
        /// <param name="query"></param>
        /// <param name="location"></param>
        /// <returns></returns>
        inline size_type result_index(const size_type query,
                const query_location location) const {
            return this->result_index(this->_idx_active_buffer, query,
                location);
        }

        /// <summary>
        /// Answer how many queries are actually issues per buffer/frame.
        /// </summary>
        /// <returns></returns>
        inline size_type queries_per_buffer(void) const {
            return 2 * this->_cnt_queries;
        }

        /// <summary>
        /// The number of buffers.
        /// </summary>
        size_type _cnt_buffers;

        /// <summary>
        /// The number of queries.
        /// </summary>
        size_type _cnt_queries;

        /// <summary>
        /// The device that we create the queries on.
        /// </summary>
        device_type _device;

        /// <summary>
        /// The heap on which the queries are allocated.
        /// </summary>
        heap_type _heap;

        /// <summary>
        /// The type of <see cref="_heap" />, which is required for reallocating
        /// a descriptor heap of different size.
        /// </summary>
        D3D12_QUERY_HEAP_TYPE _heap_type;

        /// <summary>
        /// The index of the current buffer/frame that is being queried.
        /// </summary>
        /// <remarks>
        /// The frame index is automatically rotated by the timer once the end
        /// of a frame was requested.
        /// </remarks>
        size_type _idx_active_buffer;

        /// <summary>
        /// The buffer receiving the results of the timestamp queries.
        /// </summary>
        result_buffer_type _result_buffer;
    };

}
}

#include "trrojan/d3d12/gpu_timer.inl"
