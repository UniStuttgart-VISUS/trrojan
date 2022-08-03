// <copyright file="sphere_streaming_context.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_context.h"

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
        : _batch_count(0), _batch_size(0), _data(nullptr), _total_batches(0) { }


/*
 * trrojan::d3d12::sphere_streaming_context::sphere_streaming_context
 */
trrojan::d3d12::sphere_streaming_context::sphere_streaming_context(
        const configuration& config)
    : _batch_count(config.get<unsigned int>(factor_batch_count)),
        _batch_size(config.get<unsigned int>(factor_batch_size)),
        _data(nullptr), _total_batches(0) { }


/*
 * trrojan::d3d12::sphere_streaming_context::sphere_streaming_context
 */
trrojan::d3d12::sphere_streaming_context::sphere_streaming_context(
        sphere_streaming_context&& rhs) {

}


/*
 * trrojan::d3d12::sphere_streaming_context::operator =
 */
trrojan::d3d12::sphere_streaming_context&
trrojan::d3d12::sphere_streaming_context::operator =(
        sphere_streaming_context&& rhs) {
    // TODO: insert return statement here
    throw "TODO";
}


/*
 * trrojan::d3d12::sphere_streaming_context::alloc_buffer
 */
void trrojan::d3d12::sphere_streaming_context::alloc_buffer(
        ID3D12Device *device, const std::size_t stride,
        const std::size_t pipeline_depth) {
    assert(device != nullptr);
    assert(stride > 0);
    assert(pipeline_depth > 0);
    const auto heap_size = this->_batch_count * this->_batch_size
        * stride * pipeline_depth;
    assert(heap_size > 0);

    trrojan::log::instance().write_line(log_level::debug, "Allocating {0} "
        "bytes of streaming buffer for {1} batch(es) of {2} particle(s) of "
        "size {3} in {4} frame(s) on device 0x{5:p} ...", heap_size,
        this->_batch_count, this->_batch_size, stride, pipeline_depth,
        static_cast<void *>(device));
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
