// <copyright file="sphere_streaming_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_context.h"

#include <algorithm>
#include <limits>

#include "trrojan/contains.h"
#include "trrojan/log.h"

#include "trrojan/d3d12/utilities.h"


#define _SPHERE_BENCH_DEFINE_FACTOR(f)                                         \
const char *trrojan::d3d12::sphere_streaming_context::factor_##f = #f

_SPHERE_BENCH_DEFINE_FACTOR(batch_count);
_SPHERE_BENCH_DEFINE_FACTOR(batch_size);
_SPHERE_BENCH_DEFINE_FACTOR(max_heap_size);
_SPHERE_BENCH_DEFINE_FACTOR(repeat_frame);

#undef _SPHERE_BENCH_DEFINE_FACTOR


/*
 * trrojan::d3d12::sphere_streaming_context::add_defaults
 */
void trrojan::d3d12::sphere_streaming_context::add_defaults(
        trrojan::configuration_set& configs) {
    configs.add_factor(factor::from_manifestations(factor_batch_count, 8u));
    configs.add_factor(factor::from_manifestations(factor_batch_size, 1024u));
    configs.add_factor(factor::from_manifestations(factor_max_heap_size,
        static_cast<std::uint64_t>(1024 * 1024 * 1024)));
    configs.add_factor(factor::from_manifestations(factor_repeat_frame, 0u));
}


/*
 * trrojan::d3d12::sphere_streaming_context::sphere_streaming_context
 */
trrojan::d3d12::sphere_streaming_context::sphere_streaming_context(void)
    : _batch_size(0), _cnt_stalls(0), _event(create_event(false, false)),
    _next_fence_value(0), _ready_count(0), _repeat_frame(1), _stride(0),
    _total_spheres(0), _total_batches(0) { }


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
        const std::size_t batch) const {
    if (batch >= this->_buffers.size()) {
        throw std::invalid_argument("The specified batch does not exist.");
    }
    if (this->_buffers[batch] == nullptr) {
        throw std::logic_error("A descriptor can only be obtained if the "
            "streaming buffer has been built.");
    }

    return this->_buffers[batch]->GetGPUVirtualAddress();
}


/*
 * trrojan::d3d12::sphere_streaming_context::fence_values
 */
std::pair<UINT64, UINT64>
trrojan::d3d12::sphere_streaming_context::fence_values(void) const {
    assert(this->_fence);
    return std::make_pair(this->_fence->GetCompletedValue(),
        this->_next_fence_value.load());
}


/*
 * trrojan::d3d12::sphere_streaming_context::next_batch
 */
std::size_t trrojan::d3d12::sphere_streaming_context::next_batch(void) {
    assert(this->_event != nullptr);
    assert(this->_fence != nullptr);
    std::size_t completed = 0;
    auto retval = (std::numeric_limits<std::size_t>::max)();

    // Every batch up to this fence value is reusable.
    auto finished_value = this->_fence->GetCompletedValue();
    //log::instance().write_line(log_level::debug, "Fence value for completed "
    //    "batches is {0}.", finished_value);

    // If all batches are currently in flight on the GPU, ie used for rendering,
    // we must wait for the GPU to finish to provide the caller with a batch
    // that can be used for upload.
    while (this->_ready_count < 1) {
        //log::instance().write_line(log_level::debug, "No batch is available.");
        ++this->_cnt_stalls;

        // Wait for the event to signal.
        wait_for_event(this->_event);

        // The latest finished fence has hopefully changed.
        finished_value = this->_fence->GetCompletedValue();
        //log::instance().write_line(log_level::debug, "Waited for fence to "
        //    "become {0}.", finished_value);

        // This loop performs two tasks: first, it finds a free slot that we can
        // return, and second, it counts how many free slots there are in total,
        // which we need to update the '_ready_count' member.
        this->_ready_count = 0;
        for (std::size_t i = 0; i < this->_fence_values.size(); ++i) {
            if (this->_fence_values[i] <= finished_value) {
                // We found a batch that has completed rendering.
                ++this->_ready_count;
                retval = i;
            }
        }
    }
    // At least one must be ready. Otherwise, something went really wrong when
    // setting the fence values.
    assert(this->_ready_count > 0);

    // If we came here without waiting, we need to search for the return value,
    // because we never came through the same loop above.
    if (retval > this->_fence_values.size()) {
        this->_ready_count = 0;
        for (std::size_t i = 0; i < this->_fence_values.size(); ++i) {
            if (this->_fence_values[i] <= finished_value) {
                // We found a batch that has completed rendering.
                ++this->_ready_count;
                retval = i;
            }
        }
    }

    //log::instance().write_line(log_level::debug, "Batch {0} is ready with "
    //    "fence value {1}.", retval, finished_value);

    // Make sure that we do not hand out the batch index again until the fence
    // was injected and passed in the command queue using signal_done().
    this->_fence_values[retval] = (std::numeric_limits<UINT64>::max)();
    --this->_ready_count;

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::rebuild_required
 */
bool trrojan::d3d12::sphere_streaming_context::rebuild_required(
        const std::vector<std::string>& changed) const {
    return (contains_any(changed, factor_batch_count, factor_batch_size,
        factor_repeat_frame, benchmark_base::factor_device)
        || this->_heaps.empty()
        || std::any_of(this->_heaps.begin(), this->_heaps.end(),
            [](winrt::com_ptr<ID3D12Heap> h) { return !h; }));
}


/*
 * trrojan::d3d12::sphere_streaming_context::rebuild
 */
void trrojan::d3d12::sphere_streaming_context::rebuild(ID3D12Device *device,
        const configuration& config,
        const D3D12_HEAP_TYPE heap_type,
        const D3D12_RESOURCE_STATES initial_state) {
    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device must be valid.");
    }

    const auto max_heap_size = config.get<std::uint64_t>(factor_max_heap_size);

    // Retrieve and cache the number of parallel batches and the number of
    // spheres they contain.
    assert(this->_buffers.empty());
    this->_buffers.resize(config.get<std::uint32_t>(factor_batch_count));
    this->_batch_size = config.get<decltype(this->_batch_size)>(
        factor_batch_size);
    this->_repeat_frame = config.get<decltype(this->_repeat_frame)>(
        factor_repeat_frame);
    //this->_batch_count *this->_batch_size *this->_stride;

    // Compute and cache the total number of batches.
    this->_total_batches = this->_total_spheres / this->_batch_size;
    if (this->_total_spheres % this->_batch_size != 0) {
        ++this->_total_batches;
    }

    // Allocate a single heap which we can use to create the resources.
    const auto batch_size = this->batch_size();
    if (batch_size < 1) {
        throw std::invalid_argument("The streaming heap cannot be empty. Make "
            "sure that the batch size and count are set correctly and that the "
            "data properties are set as well.");
    }

    {
        D3D12_RESOURCE_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        desc.DepthOrArraySize = 1;
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.Height = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Width = batch_size;

        auto info = device->GetResourceAllocationInfo(0, 1, &desc);

        assert(this->_heaps.empty());
        if (desc.Width > max_heap_size) {
            trrojan::log::instance().write_line(log_level::debug, "Allocating "
                "a heap of {0} bytes ({1} minimum required) of streaming "
                "buffer for each of {2} batch(es) of {3} particle(s) of size "
                "{4} on device 0x{5:p} ...", info.SizeInBytes, batch_size,
                this->batch_count(), this->_batch_size, this->_stride,
                static_cast<void *>(device));
            this->_heaps.push_back(to_winrt(create_heap(device, info,
                this->batch_count(), heap_type)));
        } else {
            trrojan::log::instance().write_line(log_level::debug, "Allocating "
                "{2} heap(s() of {0} bytes ({1} minimum required) for streaming "
                "buffer(s) for batch(es) of {3} particle(s) of size {4} on "
                "device 0x{5:p} ...", info.SizeInBytes, batch_size,
                this->batch_count(), this->_batch_size, this->_stride,
                static_cast<void *>(device));
            for (std::size_t b = 0; b < this->batch_count(); ++b) {
                this->_heaps.push_back(to_winrt(create_heap(device, info, 1,
                    heap_type)));
            }
        }

        for (std::size_t b = 0; b < this->batch_count(); ++b) {
            auto offset = b * info.SizeInBytes;
            auto heap = this->_heaps.front();

            if (this->_heaps.size() >= this->batch_count()) {
                offset = 0;
                heap = this->_heaps[b];
            }

            trrojan::log::instance().write_line(log_level::debug, "Creating a "
                "placed resource at offset {0} for batch {1} in heap "
                "0x{2:p}... ", offset, b, static_cast<void *>(heap.get()));
            auto hr = device->CreatePlacedResource(heap.get(), offset, &desc,
                initial_state, nullptr, IID_PPV_ARGS(this->_buffers[b].put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }
    }

    this->_data.resize(this->_buffers.size());
    if (heap_type == D3D12_HEAP_TYPE_UPLOAD) {
        trrojan::log::instance().write_line(log_level::debug,
            "Persistently mapping data streaming buffers ...");
        for (std::size_t b = 0; b < this->batch_count(); ++b) {
            auto hr = this->_buffers[b]->Map(0, nullptr, &this->_data[b]);
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
            trrojan::log::instance().write_line(log_level::debug, "Streaming "
                "buffer {0} mapped to 0x{1:p}.", b, this->_data[b]);
        }
    } else {
        trrojan::log::instance().write_line(log_level::debug,
            "Streaming buffers are not backed by an upload heap and therefore "
            "cannot be mapped persistently.");
        std::fill(this->_data.begin(), this->_data.end(), nullptr);
    }

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
        "Initialising fence values for {0} batches ...", this->batch_count());
    this->_fence_values.resize(this->batch_count());
    std::fill(this->_fence_values.begin(), this->_fence_values.end(), 0);

    // Reset the fence values we use. We also assume that nothing is in flight
    // while we are asked to rebuild the context, so we can assume all batches
    // to be ready for uploading data.
    this->_ready_count = this->batch_count();
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
        this->_heaps.clear();
        this->_buffers.clear();
        this->_data.clear();
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
    assert(batch < this->batch_count());
    assert(batch < this->_fence_values.size());
    assert(queue != nullptr);
    assert(this->_fence != nullptr);

    auto value = this->_fence_values[batch] = ++this->_next_fence_value;
    //log::instance().write_line(log_level::debug, "Signalling batch {0} with "
    //    "fence value {1}.", batch, value);

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


/*
 * trrojan::d3d12::sphere_streaming_context::try_next_batch
 */
std::size_t trrojan::d3d12::sphere_streaming_context::try_next_batch(void) {
    assert(this->_fence != nullptr);
    auto retval = (std::numeric_limits<std::size_t>::max)();

    // Every batch up to this fence value is reusable.
    const auto finished_value = this->_fence->GetCompletedValue();
    //log::instance().write_line(log_level::debug, "Fence is {0}.",
    //    finished_value);

    /// Search all fence values for reusable ones.
    this->_ready_count = 0;
    for (std::size_t i = 0; i < this->_fence_values.size(); ++i) {
        if (this->_fence_values[i] <= finished_value) {
            // We found a batch that has completed rendering.
            ++this->_ready_count;
            retval = i;
        }
    }

    // Make sure that we do not hand out the batch index again until the fence
    // was injected and passed in the command queue using signal_done().
    if (retval < this->_fence_values.size()) {
        this->_fence_values[retval] = (std::numeric_limits<UINT64>::max)();
        --this->_ready_count;
    }

    return retval;
}
