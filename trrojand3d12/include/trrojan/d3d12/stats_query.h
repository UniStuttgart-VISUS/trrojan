// <copyright file="stats_query.h" company="Visualisierungsinstitut der Universität Stuttgart">
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
    /// Utility class for performing pipeline statistics queries.
    /// </summary>
    class stats_query final {

    public:

        /// <summary>
        /// The type used to express buffer sizes.
        /// </summary>
        typedef UINT size_type;

        /// <summary>
        /// Represents the result of one statistics query.
        /// </summary>
        typedef D3D12_QUERY_DATA_PIPELINE_STATISTICS value_type;

        /// <summary>
        /// The identifier of the query issued by this object.
        /// </summary>
        static constexpr D3D12_QUERY_TYPE query_type
            = D3D12_QUERY_TYPE_PIPELINE_STATISTICS;

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="queries"></param>
        /// <param name="buffers"></param>
        stats_query(ID3D12Device *device, const size_type queries = 1,
            const size_t buffers = 2);

        stats_query(const stats_query&) = delete;

        stats_query(stats_query&&) = delete;

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~stats_query(void) = default;

        /// <summary>
        /// Begins the <paramref name="query" />th query within the current
        /// frame.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="query"></param>
        void begin(ID3D12GraphicsCommandList *cmd_list, const UINT query);

        /// <summary>
        /// Starts a new frame.
        /// </summary>
        void begin_frame(void);

        /// <summary>
        /// Ends the <paramref name="query" />th query within the current frame.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <param name="query"></param>
        void end(ID3D12GraphicsCommandList *cmd_list, const UINT query);

        /// <summary>
        /// Retrieve the query data for the currently active frame and return
        /// the buffer index for evaluating it.
        /// </summary>
        /// <param name="cmd_list"></param>
        /// <returns></returns>
        size_type end_frame(ID3D12GraphicsCommandList *cmd_list);

        /// <summary>
        /// Retrieve all query results of the given <paramref name="buffer "/>.
        /// </summary>
        /// <typeparam name="TIterator"></typeparam>
        /// <param name="oit"></param>
        /// <param name="buffer"></param>
        template<class TIterator>
        void evaluate(TIterator oit, const size_type buffer) const;

        /// <summary>
        /// Retrieve the <paramref name="query" />th query of the given
        /// <paramref name="buffer" />.
        /// </summary>
        /// <param name="buffer"></param>
        /// <param name="query"></param>
        /// <returns></returns>
        value_type evaluate(const size_type buffer, const UINT query);

        /// <summary>
        /// Resizes the result buffers.
        /// </summary>
        /// <param name="queries"></param>
        /// <param name="buffers"></param>
        void resize(const size_type queries, const size_type buffers = 2);

        stats_query& operator =(const stats_query&) = delete;

        stats_query& operator =(stats_query&&) = delete;

    private:

        typedef ATL::CComPtr<ID3D12Device> device_type;
        typedef ATL::CComPtr<ID3D12QueryHeap> heap_type;
        typedef ATL::CComPtr<ID3D12Resource> result_buffer_type;

        inline size_type result_index(const size_type buffer,
                const UINT query) const {
            return buffer * this->_cnt_queries + query;
        }

        inline size_type result_index(const UINT query) const {
            return this->result_index(this->_idx_active_buffer, query);
        }

        size_type _cnt_buffers;
        size_type _cnt_queries;
        device_type _device;
        heap_type _heap;
        size_type _idx_active_buffer;
        result_buffer_type _result_buffer;
    };

}
}

#include "trrojan/d3d12/stats_query.inl"
