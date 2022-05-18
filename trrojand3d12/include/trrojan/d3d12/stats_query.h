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
    /// Utility class for performing pipeline statistics queries..
    /// </summary>
    /// <remarks>
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
        /// Initialise a new instance.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="queries"></param>
        stats_query(ID3D12Device *device, const size_type queries = 1);

        /// <summary>
        /// Initialise a new instance.
        /// </summary>
        /// <param name="queue"></param>
        /// <param name="queries"></param>
        stats_query(ID3D12CommandQueue *queue, const size_type queries = 1);

        stats_query(const stats_query&) = delete;

        stats_query(stats_query&&) = delete;

        /// <summary>
        /// Finalise the instance.
        /// </summary>
        ~stats_query(void);

        stats_query& operator =(const stats_query&) = delete;

        stats_query& operator =(stats_query&&) = delete;

    private:

        typedef ATL::CComPtr<ID3D12Device> device_type;
        typedef ATL::CComPtr<ID3D12QueryHeap> heap_type;
        typedef ATL::CComPtr<ID3D12Resource> result_buffer_type;

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
        /// The buffer receiving the results of the pipeline statistics queries.
        /// </summary>
        result_buffer_type _result_buffer;
    };

}
}
