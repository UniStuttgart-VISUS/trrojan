// <copyright file="sphere_streaming_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_context.h"

#include <limits>

#include "trrojan/contains.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::sphere_streaming_context::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(batch_count);
_SPHERE_BENCH_DEFINE_FACTOR(batch_size);

#undef _SPHERE_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::sphere_streaming_context::sphere_streaming_context
 */
trrojan::d3d12::sphere_streaming_context::sphere_streaming_context(void)
        : _batch_count(0), _batch_size(0), _data(nullptr),
        _event(create_event(false, false)), _next_fence_value(0),
        _ready_count(0), _stride(0), _total_spheres(0) { }


/*
 * trrojan::d3d12::sphere_streaming_context::~sphere_streaming_context
 */
trrojan::d3d12::sphere_streaming_context::~sphere_streaming_context(void) { }


/*
 * trrojan::d3d12::sphere_streaming_context::batch_elements
 */
std::size_t trrojan::d3d12::sphere_streaming_context::batch_elements(
        const std::size_t batch) const {
    assert(batch < this->total_batches());
    auto retval = this->_batch_size;

    if (batch == this->_total_batches - 1) {
        auto remainder = this->_total_spheres % this->_batch_size;
        if (remainder > 0) {
            retval = remainder;
        }
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::descriptor
 */
D3D12_GPU_VIRTUAL_ADDRESS trrojan::d3d12::sphere_streaming_context::descriptor(
        void) const {
    if (this->_buffer == nullptr) {
        throw std::logic_error("A descriptor can only be obtained if the "
            "streaming buffer has been built.");
    }

    return this->_buffer->GetGPUVirtualAddress();
}


/*
 * trrojan::d3d12::sphere_streaming_context::descriptor
 */
D3D12_GPU_VIRTUAL_ADDRESS trrojan::d3d12::sphere_streaming_context::descriptor(
        const std::size_t batch) const {
    auto retval = this->descriptor();
    retval += this->offset(batch);
    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::frame_size
 */
std::size_t trrojan::d3d12::sphere_streaming_context::frame_size(
        void) const noexcept {
    return this->_batch_count * this->_batch_size * this->_stride;
}


/*
 * trrojan::d3d12::sphere_streaming_context::next_batch
 */
std::size_t trrojan::d3d12::sphere_streaming_context::next_batch(void) {
    assert(this->_event != nullptr);
    assert(this->_fence != nullptr);
    std::size_t completed = 0;
    std::size_t retval = 0;

    // If all batches are currently in flight on the GPU, ie used for rendering,
    // we must wait for the GPU to finish to provide the caller with a batch
    // that can be used for upload.
    if (this->_ready_count == 0) {
        // Wait for the event to signal.
        wait_for_event(this->_event);
    }

    // This loop performs two tasks: first, it finds a free slot that we can
    // return, and second, it counts how many free slots there are in total,
    // which we need to update the '_in_flight' member.
    const auto finished_value = this->_fence->GetCompletedValue();
    this->_ready_count = 0;
    for (std::size_t i = 0; i < this->_fence_values.size(); ++i) {
        if (this->_fence_values[i] <= finished_value) {
            // We found a batch that has completed rendering.
            ++this->_ready_count;
            retval = i;
        }
    }
    // At least one must be ready. Otherwise, something went really wrong when
    // setting the fence values.
    assert(this->_ready_count > 0);

    // Make sure that we do not hand out the batch index again until the fence
    // was injected and passed in the command queue using signal_done().
    this->_fence_values[retval] = (std::numeric_limits<UINT64>::max)();
    -- this->_ready_count;

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::rebuild_required
 */
bool trrojan::d3d12::sphere_streaming_context::rebuild_required(
        const std::vector<std::string>& changed) const {
    return (this->_buffer == nullptr) || (this->_data == nullptr)
        || contains_any(changed, factor_batch_count, factor_batch_size,
        benchmark_base::factor_device);
}


/*
 * trrojan::d3d12::sphere_streaming_context::rebuild
 */
void trrojan::d3d12::sphere_streaming_context::rebuild(ID3D12Device *device,
        const configuration& config) {
    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device must be valid.");
    }

    // Retrieve and cache the number of parallel batches and the number of
    // spheres they contain.
    this->_batch_count = config.get<decltype(this->_batch_count)>(
        factor_batch_count);
    this->_batch_size = config.get<decltype(this->_batch_size)>(
        factor_batch_size);

    // Compute and cache the total number of batches.
    this->_total_batches = this->_total_spheres / this->_batch_size;
    if (this->_total_spheres % this->_batch_size != 0) {
        ++this->_total_batches;
    }

    const auto heap_size = this->frame_size();
    if (heap_size < 1) {
        throw std::invalid_argument("The streaming heap cannot be empty. Make "
            "sure that the batch size and count are set correctly and that the "
            "data properties are set as well.");
    }

    trrojan::log::instance().write_line(log_level::debug, "Allocating {0} "
        "bytes of streaming buffer for {1} batch(es) of {2} particle(s) of "
        "size {3} on device 0x{5:p} ...", heap_size, this->_batch_count,
        this->_batch_size, this->_stride, static_cast<void *>(device));
    this->_buffer = to_winrt(create_upload_buffer(device, heap_size));

    trrojan::log::instance().write_line(log_level::debug,
        "Persistently mapping data streaming buffer ...");
    {
        auto hr = this->_buffer->Map(0, nullptr, &this->_data);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
    trrojan::log::instance().write_line(log_level::debug, "Streaming buffer "
        "mapped to 0x{:p}.", this->_data);

    trrojan::log::instance().write_line(log_level::debug,
        "Creating fence for staged uploads ...");
    {
        this->_fence = nullptr;
        auto hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
            ::IID_ID3D12Fence, this->_fence.put_void());
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    trrojan::log::instance().write_line(log_level::debug,
        "Initialising fence values for {0} batches ...", this->_batch_count);
    this->_fence_values.resize(this->_batch_count);
    std::fill(this->_fence_values.begin(), this->_fence_values.end(), 0);

    // Reset the fence values we use. We also assume that nothing is in flight
    // while we are asked to rebuild the context, so we can assume all batches
    // to be ready for uploading data.
    this->_ready_count = this->_batch_count;
    this->_next_fence_value = 0;
}


/*
 * trrojan::d3d12::sphere_streaming_context::reshape
 */
bool trrojan::d3d12::sphere_streaming_context::reshape(
        const std::size_t total_spheres, const std::size_t stride) {
    auto retval = (this->_stride != stride);

    if (retval) {
        // Force rebuild. The total number of spheres is irrelevant, because it
        // only affects the last batch, which we compute on the fly.
        this->_buffer = nullptr;
        this->_data = nullptr;
    }

    this->_stride = stride;
    this->_total_spheres = total_spheres;

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::signal_done
 */
void trrojan::d3d12::sphere_streaming_context::signal_done(
        const std::size_t batch, ID3D12CommandQueue *queue) {
    assert(batch < this->_batch_count);
    assert(batch < this->_fence_values.size());
    assert(queue != nullptr);
    assert(this->_fence != nullptr);

    auto value = this->_fence_values[batch] = ++this->_next_fence_value;

    {
        auto hr = queue->Signal(this->_fence.get(), value);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }

    {
        auto hr = this->_fence->SetEventOnCompletion(value, this->_event);
        if (FAILED(hr)) {
            throw ATL::CAtlException(hr);
        }
    }
}
