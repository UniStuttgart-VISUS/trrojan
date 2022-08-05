// <copyright file="sphere_streaming_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_context.h"

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
        : _batch_count(0), _batch_size(0), _data(nullptr), _stride(0),
        _total_batches(0), _total_spheres(0) { }


/*
 * trrojan::d3d12::sphere_streaming_context::batch
 */
std::pair<std::size_t, std::size_t>
trrojan::d3d12::sphere_streaming_context::batch(const std::size_t batch,
        const std::size_t frame) const {
    auto size = this->frame_size();
    auto offset = batch * size;

    if (offset + size > this->frame_size()) {
        size = this->frame_size() - offset;
    }
    assert(size % this->_stride == 0);

    offset += frame * this->frame_size();

    return std::make_pair(offset, size);
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
        const std::size_t batch, const std::size_t frame) const {
    auto retval = this->descriptor();
    retval += this->batch(batch, frame).first;
    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_context::frame_size
 */
std::size_t trrojan::d3d12::sphere_streaming_context::frame_size(void) const {
    return this->_batch_count * this->_batch_size * this->_stride;
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
        const configuration& config, const std::size_t pipeline_depth) {
    if (device == nullptr) {
        throw std::invalid_argument("The Direct3D device must be valid.");
    }

    this->_batch_count = config.get<decltype(this->_batch_count)>(
        factor_batch_count);
    this->_batch_size = config.get<decltype(this->_batch_size)>(
        factor_batch_size);

    const auto heap_size = this->frame_size() * pipeline_depth;
    if (heap_size < 1) {
        throw std::invalid_argument("The streaming heap cannot be empty. Make "
            "sure that the batch size and count are set correctly and that the "
            "data properties are set as well.");
    }

    trrojan::log::instance().write_line(log_level::debug, "Allocating {0} "
        "bytes of streaming buffer for {1} batch(es) of {2} particle(s) of "
        "size {3} in {4} frame(s) on device 0x{5:p} ...", heap_size,
        this->_batch_count, this->_batch_size, this->_stride,
        pipeline_depth, static_cast<void *>(device));
    this->_buffer = create_upload_buffer(device, heap_size);

    trrojan::log::instance().write_line(log_level::debug,
        "Mapping data streaming buffer ...");
    auto hr = this->_buffer->Map(0, nullptr, &this->_data);
    if (FAILED(hr)) {
        throw ATL::CAtlException(hr);
    }
    trrojan::log::instance().write_line(log_level::debug, "Streaming buffer "
        "mapped to 0x{:p}.", this->_data);
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
