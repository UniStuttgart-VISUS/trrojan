// <copyright file="gpu_timer.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/gpu_timer.h"


//   ThrowIfFailed(m_directCommandQueues[i]->GetTimestampFrequency(&m_directCommandQueueTimestampFrequencies[i]));


/*
 * trrojan::d3d12::gpu_timer::get_clock_calibration
 */
void trrojan::d3d12::gpu_timer::get_clock_calibration(
        ID3D12CommandQueue *queue,
        value_type& gpu_timestamp,
        value_type& cpu_timestamp) {
    if (queue == nullptr) {
        throw std::invalid_argument("A valid command queue must be provided to "
            "obtain its clock calibration.");
    }

    auto hr = queue->GetClockCalibration(&gpu_timestamp, &cpu_timestamp);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
}


/*
 * trrojan::d3d12::gpu_timer::get_timestamp_frequency
 */
trrojan::d3d12::gpu_timer::value_type
trrojan::d3d12::gpu_timer::get_timestamp_frequency(
        ID3D12CommandQueue *queue) {
    if (queue == nullptr) {
        throw std::invalid_argument("A valid command queue is required to "
            "determine its timestamp frequency");
    }

    value_type retval;

    auto hr = queue->GetTimestampFrequency(&retval);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }

    return retval;
}


/*
 * trrojan::d3d12::gpu_timer::to_milliseconds
 */
trrojan::d3d12::gpu_timer::millis_type
trrojan::d3d12::gpu_timer::to_milliseconds(
        const difference_type value, const value_type frequency) {
    auto v = static_cast<millis_type>(value);
    auto s = trrojan::constants<millis_type>::millis_per_second;
    auto f = static_cast<millis_type>(frequency);
    return ((v * s) / f);
}


/*
 * trrojan::d3d12::gpu_timer::to_milliseconds
 */
trrojan::d3d12::gpu_timer::millis_type
trrojan::d3d12::gpu_timer::to_milliseconds(
        const value_type value, const value_type frequency) {
    auto v = static_cast<millis_type>(value);
    auto s = trrojan::constants<millis_type>::millis_per_second;
    auto f = static_cast<millis_type>(frequency);
    return ((v * s) / f);
}


/*
 * trrojan::d3d12::gpu_timer::gpu_timer
 */
trrojan::d3d12::gpu_timer::gpu_timer(ID3D12Device *device,
        const size_type queries, const size_type buffers,
        const D3D12_QUERY_HEAP_TYPE heap_type)
        : _device(device), _heap_type(heap_type) {
    if (this->_device == nullptr) {
        throw std::invalid_argument("The device cannot be null.");
    }

    this->resize(queries, buffers);
    assert(this->_cnt_buffers = buffers);
    assert(this->_cnt_queries = queries);
    assert(this->_idx_active_buffer == 0);
}


/*
 * trrojan::d3d12::gpu_timer::gpu_timer
 */
trrojan::d3d12::gpu_timer::gpu_timer(ID3D12CommandQueue *queue,
        const size_type queries, const size_type buffers) {
    if (queue == nullptr) {
        throw std::invalid_argument("A valid command queue is required to "
            "intialise a gpu_timer.");
    }

    ATL::CComPtr<ID3D12Device> device;
    {
        auto hr = queue->GetDevice(IID_PPV_ARGS(&this->_device));
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    switch (queue->GetDesc().Type) {
        case D3D12_COMMAND_LIST_TYPE_DIRECT:
        case D3D12_COMMAND_LIST_TYPE_COMPUTE:
            this->_heap_type = D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
            break;

        case D3D12_COMMAND_LIST_TYPE_COPY:
            this->_heap_type = D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP;
            break;

        default:
            throw std::invalid_argument("Only direct, compute and copy queues "
                "can be used to issue timestamp queries.");
    }

    this->resize(queries, buffers);
    assert(this->_cnt_buffers = buffers);
    assert(this->_cnt_queries = queries);
    assert(this->_idx_active_buffer == 0);
}


/*
 * trrojan::d3d12::gpu_timer::~gpu_timer
 */
trrojan::d3d12::gpu_timer::~gpu_timer(void) { }


/*
 * trrojan::d3d12::gpu_timer::end
 */
void trrojan::d3d12::gpu_timer::end(ID3D12GraphicsCommandList *cmd_list,
        const size_type query) {
    assert(cmd_list != nullptr);
    cmd_list->EndQuery(this->_heap,
        D3D12_QUERY_TYPE_TIMESTAMP,
        this->result_index(query, query_location::end));
}


/*
 * trrojan::d3d12::gpu_timer::end_frame
 */
trrojan::d3d12::gpu_timer::size_type trrojan::d3d12::gpu_timer::end_frame(
        ID3D12GraphicsCommandList *cmd_list) {
    assert(cmd_list != nullptr);
    const auto first = std::min(query_location::start, query_location::end);
    const auto index = this->result_index(0, first);
    const auto queries = this->queries_per_buffer();
    const auto offset = index * sizeof(value_type);
    cmd_list->ResolveQueryData(this->_heap, D3D12_QUERY_TYPE_TIMESTAMP,
        index, queries, this->_result_buffer, offset);
    return this->_idx_active_buffer;
}


/*
 * trrojan::d3d12::gpu_timer::evaluate
 */
void trrojan::d3d12::gpu_timer::evaluate(value_type& outStart,
        value_type& outEnd, const size_type buffer,
        const size_type query) const {
    if (buffer >= this->_cnt_buffers) {
        throw std::invalid_argument("The requested buffer is out of range.");
    }
    if (query >= this->_cnt_queries) {
        throw std::invalid_argument("The requested query index is out of "
            "range.");
    }

    //std::vector<difference_type> xxx;
    //this->evaluate(std::back_inserter(xxx), buffer);

    const auto off = std::min(query_location::start, query_location::end);
    void *data;
    D3D12_RANGE range;

    range.Begin = this->result_index(buffer, query, off) * sizeof(value_type);
    range.End = this->result_index(buffer, query + 1, off) * sizeof(value_type);

    {
        auto hr = this->_result_buffer->Map(0, &range, &data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    outStart = *(reinterpret_cast<value_type *>(data)
        + static_cast<size_type>(query_location::start));
    outEnd = *(reinterpret_cast<value_type *>(data)
        + static_cast<size_type>(query_location::end));

    ::ZeroMemory(&range, sizeof(range));
    this->_result_buffer->Unmap(0, &range);
}


/*
 * trrojan::d3d12::gpu_timer::evaluate
 */
trrojan::d3d12::gpu_timer::difference_type trrojan::d3d12::gpu_timer::evaluate(
        const size_type buffer, const size_type query) const {
    value_type start, end;
    this->evaluate(start, end, buffer, query);
    return (end - start);
}


/*
 * trrojan::d3d12::gpu_timer::resize
 */
void trrojan::d3d12::gpu_timer::resize(const size_type queries,
        const size_type buffers) {
    assert(this->_device != nullptr);
    if (queries < 1) {
        throw std::invalid_argument("A non-zero number of queries must be "
            "allocated for a gpu_timer.");
    }
    if (buffers < 1) {
        throw std::invalid_argument("A non-zero number of buffers must be "
            "specified for a gpu_timer.");
    }

    // Compute the number of descriptors we actually need, which are two per
    // query (begin and end) per buffer.
    const auto cnt = 2 * buffers * queries;

    // Clear old buffer.
    this->_heap = nullptr;
    this->_result_buffer = nullptr;

    // Allocate new buffer for specified number of queries as in
    // https://github.com/microsoft/DirectX-Graphics-Samples/blob/164b072185a5360c43c5f0b64e2f672b7f423f95/Samples/Desktop/D3D12HeterogeneousMultiadapter/src/D3D12HeterogeneousMultiadapter.cpp#L229
    D3D12_QUERY_HEAP_DESC desc;
    ::ZeroMemory(&desc, sizeof(desc));
    desc.Count = cnt;
    desc.Type = this->_heap_type;

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


/*
 * trrojan::d3d12::gpu_timer::start
 */
void trrojan::d3d12::gpu_timer::start(ID3D12GraphicsCommandList *cmd_list,
        const size_type query) {
    assert(cmd_list != nullptr);
    cmd_list->EndQuery(this->_heap,
        D3D12_QUERY_TYPE_TIMESTAMP,
        this->result_index(query, query_location::start));
}


/*
 * trrojan::d3d12::gpu_timer::start_frame
 */
void trrojan::d3d12::gpu_timer::start_frame(void) {
    this->_idx_active_buffer= ++this->_idx_active_buffer % this->_cnt_buffers;
}


// https://github.com/microsoft/DirectX-Graphics-Samples/blob/164b072185a5360c43c5f0b64e2f672b7f423f95/Samples/Desktop/D3D12HeterogeneousMultiadapter/src/D3D12HeterogeneousMultiadapter.cpp#L843
#if false
const UINT oldestFrameIndex = m_frameIndex;
assert(m_frameFenceValues[oldestFrameIndex] <= m_frameFence->GetCompletedValue());

// Get the timestamp values from the result buffers.
D3D12_RANGE readRange = {};
const D3D12_RANGE emptyRange = {};

UINT64 *ppMovingAverage[] = { m_drawTimes, m_blurTimes };
for (UINT i = 0; i < GraphicsAdaptersCount; i++)
{
    readRange.Begin = 2 * oldestFrameIndex * sizeof(UINT64);
    readRange.End = readRange.Begin + 2 * sizeof(UINT64);

    void *pData = nullptr;
    ThrowIfFailed(m_timestampResultBuffers[i]->Map(0, &readRange, &pData));

    const UINT64 *pTimestamps = reinterpret_cast<UINT64 *>(static_cast<UINT8 *>(pData) + readRange.Begin);
    const UINT64 timeStampDelta = pTimestamps[1] - pTimestamps[0];

    // Unmap with an empty range (written range).
    m_timestampResultBuffers[i]->Unmap(0, &emptyRange);

    // Calculate the GPU execution time in microseconds.
    const UINT64 gpuTimeUS = (timeStampDelta * 1000000) / m_directCommandQueueTimestampFrequencies[i];
    ppMovingAverage[i][m_currentTimesIndex] = gpuTimeUS;
}

// Move to the next index.
m_currentTimesIndex = (m_currentTimesIndex + 1) % MovingAverageFrameCount;
    }
#endif


