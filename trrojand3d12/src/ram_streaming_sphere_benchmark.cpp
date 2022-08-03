// <copyright file="ram_streaming_sphere_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/ram_streaming_sphere_benchmark.h"


/*
 * ...::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark
 */
trrojan::d3d12::ram_streaming_sphere_benchmark::ram_streaming_sphere_benchmark(
    void) : streaming_sphere_benchmark_base("ram-stream-sphere-renderer") { }


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::ram_streaming_sphere_benchmark::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Let the base class compute how many descriptors we need for emitting one
    // draw call.
    auto retval = streaming_sphere_benchmark_base::count_descriptor_tables(
        shader_code, false);

    // All of the above is required for each batch that can run in parallel.
    retval *= this->batch_count();

    if (include_root) {
        ++retval;
    }

    return retval;
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::on_batch_required
 */
std::size_t trrojan::d3d12::ram_streaming_sphere_benchmark::on_batch_required(
        const std::size_t next_batch, const std::size_t batch_size,
        void *stream_chunk) {
    auto retval = batch_size * this->get_stride();
    auto offset = next_batch * retval;

    assert(offset < this->_buffer.size());
    if (offset + retval > this->_buffer.size()) {
        retval = this->_buffer.size() - offset;
    }

    ::memcpy(stream_chunk, this->_buffer.data() + offset, retval);

    assert(retval % this->get_stride() == 0);
    retval /= this->get_stride();

    return retval;
}


/*
 * trrojan::d3d12::ram_streaming_sphere_benchmark::on_data_changed
 */
void trrojan::d3d12::ram_streaming_sphere_benchmark::on_data_changed(
        d3d12::device& device, const shader_id_type shader_code,
        const configuration& config) {
    trrojan::log::instance().write_line(log_level::information, "Loading "
        "\"{}\" for streaming from main memory ...",
        config.get<std::string>(factor_data_set));
    this->load_data([this](const UINT64 size) {
        this->_buffer.resize(size);
        return this->_buffer.data();
    }, shader_code, config);
}
