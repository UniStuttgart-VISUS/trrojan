// <copyright file="sphere_streaming_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_benchmark.h"

#include "trrojan/text.h"

#include "trrojan/d3d12/measurement_context.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"


/*
 * trrojan::d3d12::sphere_streaming_benchmark::factor_streaming_method
 */
const std::string
trrojan::d3d12::sphere_streaming_benchmark::factor_streaming_method
    = "streaming_method";


/*
 * trrojan::d3d12::sphere_streaming_benchmark::streaming_method_io_ring
 */
const std::string
trrojan::d3d12::sphere_streaming_benchmark::streaming_method_io_ring
    = "io_ring";


/*
 * trrojan::d3d12::sphere_streaming_benchmark::streaming_method_memory_mapping
 */
const std::string
trrojan::d3d12::sphere_streaming_benchmark::streaming_method_memory_mapping
    = "memory_mapping";



/*
 * trrojan::d3d12::sphere_streaming_benchmark::streaming_method_ram
 */
const std::string
trrojan::d3d12::sphere_streaming_benchmark::streaming_method_ram = "ram";



/*
 * trrojan::d3d12::sphere_streaming_benchmark::streaming_method_read_file
 */
const std::string
trrojan::d3d12::sphere_streaming_benchmark::streaming_method_read_file
    = "read_file";



/*
 * ...::sphere_streaming_benchmark::sphere_streaming_benchmark
 */
trrojan::d3d12::sphere_streaming_benchmark::sphere_streaming_benchmark(
        void) : sphere_benchmark_base("stream-sphere-renderer"),
        _file_view(nullptr) {
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_count, 8u));
    this->_default_configs.add_factor(factor::from_manifestations(
        sphere_streaming_context::factor_batch_size, 1024u));
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_streaming_method, { streaming_method_ram,
        streaming_method_memory_mapping }));
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::check_stream_changed
 */
bool trrojan::d3d12::sphere_streaming_benchmark::check_stream_changed(
        d3d12::device& device, const configuration& config,
        const std::vector<std::string> &changed) {
    const auto retval = this->_stream.rebuild_required(changed);

    if (retval) {
        log::instance().write_line(log_level::information, "(Re-) Building GPU "
            "stream on device 0x{0:p} ...",
            static_cast<void *>(device.d3d_device().p));
        this->_stream.rebuild(device.d3d_device(), config);

        // We need an allocator for each batch we are rendering. In contrast to
        // the base class, we do not create allocators on a per-frame basis, but
        // on a per-batch basis. This is possible, because the pipeline does not
        // change between frames in the streaming benchmarks. We just swap what
        // is in the persistenly mapped data buffer, and we do not care whether
        // we swap this content because of moving to the next batch or because
        // of moving to the next frame. The only command lists that require
        // special handling are the one at the begin of the frame and the one
        // at the end of the frame, because these need to transition the render
        // target from present to rendering state and vice versa.
        this->_direct_cmd_allocators.clear();
        create_command_allocators(this->_direct_cmd_allocators,
            device.d3d_device(),
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            this->_stream.batch_count() + 2);
    }

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::count_descriptor_tables
 */
UINT trrojan::d3d12::sphere_streaming_benchmark::count_descriptor_tables(
        const shader_id_type shader_code, const bool include_root) const {
    // Let the base class compute how many descriptors we need for emitting one
    // draw call.
    auto retval = sphere_benchmark_base::count_descriptor_tables(shader_code,
        include_root);

    // All of the above is required for each batch that can run in parallel.
    retval *= static_cast<UINT>(this->_stream.batch_count());

    return retval;
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::on_device_switch
 */
void trrojan::d3d12::sphere_streaming_benchmark::on_device_switch(
        device& device) {
    assert(device.d3d_device() != nullptr);
    sphere_benchmark_base::on_device_switch(device);
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::on_run
 */
trrojan::result trrojan::d3d12::sphere_streaming_benchmark::on_run(
        d3d12::device& device,
        const configuration& config,
        power_collector::pointer& power_collector,
        const std::vector<std::string>& changed) {
    const auto method = config.get<std::string>(factor_streaming_method);

    if (trrojan::iequals(method, streaming_method_ram)) {
        return this->on_run([this](const UINT64 size) {
            this->_buffer.resize(size);
            return this->_buffer.data();
        }, [this](void *d, const std::size_t o, const std::size_t l) {
            ::memcpy(d, this->_buffer.data() + o, l);
        }, device, config, power_collector, changed);

    } else if (trrojan::iequals(method, streaming_method_memory_mapping)) {
        return this->on_run([this](const UINT64 size) {
            auto path = create_temp_file("tssb");
            this->_file.attach(::CreateFileA(path.c_str(),
                GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
            if (!this->_file) {
                throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
            }

            this->_file_mapping.attach(::CreateFileMappingA(this->_file.get(),
                nullptr, PAGE_READWRITE, 0, static_cast<DWORD>(size), nullptr));
            if (!this->_file_mapping) {
                throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
            }

            this->_file_view = ::MapViewOfFile(this->_file_mapping.get(),
                FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, size);
            if (this->_file_view == nullptr) {
                throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
            }

            return this->_file_view;
        }, [this](void *d, const std::size_t o, const std::size_t l) {
            assert(this->_file_view != nullptr);
            ::memcpy(d, static_cast<std::uint8_t *>(this->_file_view) + o, l);
        }, device, config, power_collector, changed);

    } else {
        trrojan::log::instance().write_line(log_level::error, "The sphere "
            "streaming benchmark does not support a method named \"{0}\".",
            method);
        throw std::invalid_argument("The specified streaming method is not "
            "supported.");
    }
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::set_vertex_buffer
 */
void trrojan::d3d12::sphere_streaming_benchmark::set_vertex_buffer(
        ID3D12GraphicsCommandList *cmd_list,
        const shader_id_type shader_code,
        const std::size_t batch) {
    assert(cmd_list != nullptr);
    if (!is_technique(shader_code, SPHERE_TECHNIQUE_USE_SRV)) {
        D3D12_VERTEX_BUFFER_VIEW desc;
        desc.BufferLocation = this->_stream.descriptor(batch);
        desc.SizeInBytes = this->_stream.batch_size(0);
        desc.StrideInBytes = this->_data.stride();

#if (defined(DEBUG) || defined(_DEBUG))
        log::instance().write_line(log_level::debug, "Rendering technique uses "
            "vertex buffer. Setting 0x{0:x} ...", desc.BufferLocation);
#endif /* (defined(DEBUG) || defined(_DEBUG)) */
        cmd_list->IASetVertexBuffers(0, 1, &desc);
    }
}
