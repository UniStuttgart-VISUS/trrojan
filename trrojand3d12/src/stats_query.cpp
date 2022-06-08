// <copyright file="stats_query.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/stats_query.h"


/*
 * trrojan::d3d12::stats_query::stats_query
 */
trrojan::d3d12::stats_query::stats_query(ID3D12Device *device,
        const size_type queries, const size_t buffers) : _device(device) {
    if (this->_device == nullptr) {
        throw std::invalid_argument("The device cannot be null.");
    }

    this->resize(queries, buffers);
    assert(this->_cnt_buffers = buffers);
    assert(this->_cnt_queries = queries);
    assert(this->_idx_active_buffer == 0);
}


/*
 * trrojan::d3d12::stats_query::begin
 */
void trrojan::d3d12::stats_query::begin(ID3D12GraphicsCommandList *cmd_list,
        const UINT query) {
    assert(cmd_list != nullptr);
    assert(this->_heap != nullptr);
    auto index = this->result_index(query);
    cmd_list->BeginQuery(this->_heap, stats_query::query_type, index);
}

/*
 * trrojan::d3d12::stats_query::begin_frame
 */
void trrojan::d3d12::stats_query::begin_frame(void) {
    this->_idx_active_buffer = ++this->_idx_active_buffer % this->_cnt_buffers;
}


/*
 * trrojan::d3d12::stats_query::end
 */
void trrojan::d3d12::stats_query::end(ID3D12GraphicsCommandList *cmd_list,
        const UINT query) {
    assert(cmd_list != nullptr);
    assert(this->_heap != nullptr);
    auto index = this->result_index(query);
    cmd_list->EndQuery(this->_heap, stats_query::query_type, index);
}


/*
 * trrojan::d3d12::stats_query::end_frame
 */
trrojan::d3d12::stats_query::size_type trrojan::d3d12::stats_query::end_frame(
        ID3D12GraphicsCommandList *cmd_list) {
    assert(cmd_list != nullptr);
    const auto index = this->result_index(0);
    const auto offset = index * sizeof(value_type);
    cmd_list->ResolveQueryData(this->_heap, stats_query::query_type, index,
        this->_cnt_queries, this->_result_buffer, offset);
    return this->_idx_active_buffer;
}


/*
 * trrojan::d3d12::stats_query::evaluate
 */
trrojan::d3d12::stats_query::value_type trrojan::d3d12::stats_query::evaluate(
        const size_type buffer, const UINT query) {
    if (buffer >= this->_cnt_buffers) {
        throw std::invalid_argument("The requested buffer is out of range.");
    }
    if (query >= this->_cnt_queries) {
        throw std::invalid_argument("The requested query index is out of "
            "range.");
    }

    void *data;
    D3D12_RANGE range;

    range.Begin = this->result_index(buffer, 0) * sizeof(value_type);
    range.End = this->result_index(buffer, this->_cnt_queries)
        * sizeof(value_type);

    {
        auto hr = this->_result_buffer->Map(0, &range, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    auto results = reinterpret_cast<value_type *>(
        static_cast<std::uint8_t *>(data) + range.Begin);
    auto retval = results[query];

    ::ZeroMemory(&range, sizeof(range));
    this->_result_buffer->Unmap(0, &range);

    return retval;
}


/*
 * trrojan::d3d12::stats_query::resize
 */
void trrojan::d3d12::stats_query::resize(const size_type queries,
        const size_type buffers) {
    assert(this->_device != nullptr);
    if (queries < 1) {
        throw std::invalid_argument("A non-zero number of queries must be "
            "allocated for a stats_query.");
    }
    if (buffers < 1) {
        throw std::invalid_argument("A non-zero number of buffers must be "
            "specified for a stats_query.");
    }

    // Compute the number of descriptors we actually need, which are two per
    // query (begin and end) per buffer.
    const auto cnt = buffers * queries;

    // Clear old buffer.
    this->_heap = nullptr;
    this->_result_buffer = nullptr;

    D3D12_QUERY_HEAP_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Count = cnt;
    desc.Type = D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;

    D3D12_HEAP_PROPERTIES props;
    ::ZeroMemory(&props, sizeof(props));
    props.Type = D3D12_HEAP_TYPE_READBACK;

    D3D12_RESOURCE_DESC res;
    ::ZeroMemory(&res, sizeof(res));
    res.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    res.Width = cnt * sizeof(value_type);
    res.Height = 1;
    res.DepthOrArraySize = 1;
    res.MipLevels = 1;
    res.SampleDesc.Count = 1;
    res.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    {
        auto hr = this->_device->CreateCommittedResource(&props,
            D3D12_HEAP_FLAG_NONE,
            &res,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(&this->_result_buffer));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->_device->CreateQueryHeap(&desc,
            IID_PPV_ARGS(&this->_heap));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    // Start at the begin, because all data have been invalidated.
    this->_cnt_buffers = buffers;
    this->_cnt_queries = queries;
    this->_idx_active_buffer = 0;
}
