// <copyright file="sphere_streaming_benchmark.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/d3d12/sphere_streaming_benchmark.h"

#include "trrojan/on_exit.h"
#include "trrojan/text.h"

#include "trrojan/d3d12/dstorage_configuration.h"
#include "trrojan/d3d12/measurement_context.h"
#include "trrojan/d3d12/utilities.h"

#include "sphere_techniques.h"


#define _STRM_BENCH_DEFINE_FACTOR(f)                                           \
const char *trrojan::d3d12::sphere_streaming_benchmark::factor_##f = #f

_STRM_BENCH_DEFINE_FACTOR(queue_priority);
_STRM_BENCH_DEFINE_FACTOR(streaming_method);

#undef _STRM_BENCH_DEFINE_FACTOR


#define _STRM_BENCH_DEFINE_METHOD(m)                                           \
const std::string trrojan::d3d12::sphere_streaming_benchmark::streaming_method_##m(#m)

_STRM_BENCH_DEFINE_METHOD(batch_memory_mapping);
_STRM_BENCH_DEFINE_METHOD(dstorage);
_STRM_BENCH_DEFINE_METHOD(dstorage_memcpy);
_STRM_BENCH_DEFINE_METHOD(io_ring);
_STRM_BENCH_DEFINE_METHOD(memory_mapping);
_STRM_BENCH_DEFINE_METHOD(persistent_memory_mapping);
_STRM_BENCH_DEFINE_METHOD(ram);
_STRM_BENCH_DEFINE_METHOD(read_file);

#undef _STRM_BENCH_DEFINE_METHOD


/*
 * ...::sphere_streaming_benchmark::sphere_streaming_benchmark
 */
trrojan::d3d12::sphere_streaming_benchmark::sphere_streaming_benchmark(
        void)
    : sphere_benchmark_base("stream-sphere-renderer"),
        _allocation_granularity(0), _cnt_remap(0), _file_view(nullptr),
        _mapped_range(std::make_pair(0, 0)) {
    dstorage_configuration::add_defaults(this->_default_configs);
    sphere_streaming_context::add_defaults(this->_default_configs);
    this->_default_configs.add_factor(factor::from_manifestations(
        factor_streaming_method, {
            streaming_method_batch_memory_mapping,
            streaming_method_ram,
            streaming_method_memory_mapping,
            streaming_method_persistent_memory_mapping,
            streaming_method_dstorage,
            streaming_method_dstorage_memcpy
        }));

    // Determine the allocation granularity that we need to map the file.
    {
        SYSTEM_INFO si;
        ::GetSystemInfo(&si);
        this->_allocation_granularity = si.dwAllocationGranularity;
    }

#if defined(TRROJAN_WITH_DSTORAGE)
    this->_dstorage_fence_value = 0;
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
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
        this->_stream.rebuild(device.d3d_device(), config,
            D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

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
    const auto folder = config.get<std::string>(
        dstorage_configuration::factor_staging_directory);
    const auto method = config.get<std::string>(factor_streaming_method);
    const auto priority = config.get<std::uint32_t>(factor_queue_priority);
    const auto repeat = config.get<std::uint32_t>(
        sphere_streaming_context::factor_repeat_frame);

    if (trrojan::iequals(method, streaming_method_ram)) {
        return this->on_run([this](const UINT64 size) {
            // Provide our own in-memory buffer to receive the data.
            this->_buffer.resize(size);
            return this->_buffer.data();
        }, [](const UINT64 size) {
        }, [this](void *d, const std::size_t o, const std::size_t l) {
            // Deliver from in-memory buffer.
            ::memcpy(d, this->_buffer.data() + o, l);
        }, device, config, power_collector, changed);

    } else if (trrojan::iequals(method, streaming_method_memory_mapping)) {
#if defined(TRROJAN_FOR_UWP)
        throw std::logic_error("The requested streaming method is unsupported "
            "on UWP.");
#else /* defined(TRROJAN_FOR_UWP) */
        return this->on_run([this, &folder, repeat](const UINT64 size) {
            // Copy the raw data to a memory-mapped file such that we can
            // seek like in the in-memory file. This is required as the
            // generated data do not come from a file and therefore must be
            // persisted for the benchmark.
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);
            this->_file_mapping = create_file_mapping(this->_file,
                PAGE_READONLY, size * (1 + repeat));

        }, [this](void *d, const std::size_t o, const std::size_t l) {
            // Deliver from mapped file. Note that 'o' can be an arbitrary
            // offset, so we must manually make sure that we align it properly
            // and increase the size of the mapping if the offset does not
            // start at the expected boundary.
            const auto pad = (o % this->_allocation_granularity);
            auto s = map_view_of_file(this->_file_mapping, FILE_MAP_READ, 
                o - pad, l + pad);
            on_exit([s](void) {::UnmapViewOfFile(s); });
            ::memcpy(d, static_cast<std::uint8_t *>(s) + pad, l);
            ++this->_cnt_remap;

        }, device, config, power_collector, changed);
#endif /* defined(TRROJAN_FOR_UWP) */

    } else if (trrojan::iequals(method,
            streaming_method_persistent_memory_mapping)) {
#if defined(TRROJAN_FOR_UWP)
        throw std::logic_error("The requested streaming method is unsupported "
            "on UWP.");
#else /* defined(TRROJAN_FOR_UWP) */
        return this->on_run([this, &folder, repeat](const UINT64 size) {
            // Copy the raw data to a memory-mapped file such that we can
            // seek like in the in-memory file. This is required as the
            // generated data do not come from a file and therefore must be
            // persisted for the benchmark.
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);
            this->_file_mapping = create_file_mapping(this->_file,
                PAGE_READONLY, size * (1 + repeat));
            this->_file_view = map_view_of_file(this->_file_mapping,
                FILE_MAP_READ, 0, size * (1 + repeat));

        }, [this](void *d, const std::size_t o, const std::size_t l) {
            assert(this->_file_view != nullptr);
            ::memcpy(d, static_cast<std::uint8_t *>(this->_file_view) + o, l);

        }, device, config, power_collector, changed);
#endif /* defined(TRROJAN_FOR_UWP) */

    } else if (trrojan::iequals(method,
            streaming_method_batch_memory_mapping)) {
#if defined(TRROJAN_FOR_UWP)
        throw std::logic_error("The requested streaming method is unsupported "
            "on UWP.");
#else /* defined(TRROJAN_FOR_UWP) */
        return this->on_run([this, &folder, repeat](const UINT64 size) {
            // Copy the raw data to a memory-mapped file such that we can
            // seek like in the in-memory file. This is required as the
            // generated data do not come from a file and therefore must be
            // persisted for the benchmark.
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);
            this->_file_mapping = create_file_mapping(this->_file,
                PAGE_READONLY, size * (1 + repeat));

        }, [this, repeat](void *d, const std::size_t o, const std::size_t l) {
            // Find out whether the part we need is already mapped and if
            // not, map it.
            if ((this->_file_view == nullptr)
                    || (o < this->_mapped_range.first)
                    || (o + l > this->_mapped_range.second)) {
                if (this->_file_view != nullptr) {
                    ::UnmapViewOfFile(this->_file_view);
                }

                ULARGE_INTEGER end;
                end.LowPart = ::GetFileSize(this->_file.get(), &end.HighPart);

                const auto pad = o % this->_allocation_granularity;
                this->_mapped_range.first = o - pad;
                this->_mapped_range.second = this->_mapped_range.first
                    + this->_stream.batch_count() * l + pad;
                if (this->_mapped_range.second > end.QuadPart) {
                    this->_mapped_range.second = end.QuadPart;
                }

                this->_file_view = map_view_of_file(this->_file_mapping,
                    FILE_MAP_READ, this->_mapped_range.first,
                    this->_mapped_range.second - this->_mapped_range.first);
                ++this->_cnt_remap;
            }

            const auto ds = o - this->_mapped_range.first;
            ::memcpy(d, static_cast<std::uint8_t *>(this->_file_view) + ds, l);
        }, device, config, power_collector, changed);
#endif /* defined(TRROJAN_FOR_UWP) */

    } else if (trrojan::iequals(method, streaming_method_read_file)) {
#if defined(TRROJAN_FOR_UWP)
        throw std::logic_error("The requested streaming method is unsupported "
            "on UWP.");
#else /* defined(TRROJAN_FOR_UWP) */
        return this->on_run([this, &folder, repeat](const UINT64 size) {
            // As for the memory mapping technique, we must make sure that the
            // data are actually in a file, regardless of whether they come from
            // MMPLD or from the generator.
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            // Remove the file mapping as we do not know whether that would
            // impact other kinds of I/O.
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);

        }, [this](void *d, const std::size_t o, const std::size_t l) {
            LARGE_INTEGER position;
            position.QuadPart = o;

            if (!::SetFilePointerEx(this->_file.get(), position, nullptr,
                    FILE_BEGIN)) {
                throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
            }

            auto dst = static_cast<std::uint8_t *>(d);
            auto rem = l;
            DWORD read = 0;

            while (rem > 0) {
                if (!::ReadFile(this->_file.get(), dst, rem, &read, 0)) {
                    throw ATL::CAtlException(HRESULT_FROM_WIN32(
                        ::GetLastError()));
                }

                assert(read <= rem);
                dst += read;
                rem -= read;
            }
        }, device, config, power_collector, changed);
#endif /* defined(TRROJAN_FOR_UWP) */

    } else if (trrojan::iequals(method, streaming_method_dstorage)) {
#if defined(TRROJAN_WITH_DSTORAGE)
        // Lazily create the event for waiting on the I/O to complete.
        if (!this->_dstorage_event) {
            this->_dstorage_event.attach(::CreateEvent(nullptr, FALSE, FALSE,
                nullptr));
        }
        if (!this->_dstorage_event) {
            throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
        }

        // Lazily create the fence to wait for the operation to complete.
        if (!this->_dstorage_fence) {
            this->_dstorage_fence_value = 0;
            auto hr = device.d3d_device()->CreateFence(
                this->_dstorage_fence_value, D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(this->_dstorage_fence.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            dstorage_configuration c(config);

            // Lazily create the factory.
            if (!this->_dstorage_factory) {
                this->_dstorage_factory = c.create_factory();
            }

            // Apply the configuration.
            c.apply(this->_dstorage_factory, changed);
        }

        // Lazily create the queue.
        if (contains(changed, factor_queue_priority)) {
            this->_dstorage_queue = nullptr;
        }

        if (!this->_dstorage_queue) {
            DSTORAGE_QUEUE_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.Capacity = static_cast<UINT16>(this->pipeline_depth());
            if (desc.Capacity > DSTORAGE_MAX_QUEUE_CAPACITY) {
                desc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
            } else if (desc.Capacity < DSTORAGE_MIN_QUEUE_CAPACITY) {
                desc.Capacity = DSTORAGE_MIN_QUEUE_CAPACITY;
            }
            desc.Priority = static_cast<DSTORAGE_PRIORITY>(priority);
            desc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
            desc.Device = device.d3d_device();

            auto hr = this->_dstorage_factory->CreateQueue(&desc,
                IID_PPV_ARGS(this->_dstorage_queue.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        return this->on_run([this, &folder, repeat](const UINT64 size) {
            // As for the memory mapping technique, we must make sure that the
            // data are actually in a file, regardless of whether they come from
            // MMPLD or from the generator.
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            // Remove the file mapping as we cannot open DirectStorage if any
            // other file handle is open on the temporary file.
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);
            this->_file_mapping.close();

            const auto path = get_file_path(this->_file.get());
            this->_file.close();

            auto hr = this->_dstorage_factory->OpenFile(path.c_str(),
                IID_PPV_ARGS(this->_dstorage_file.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

        }, [this](void *d, const std::size_t o, const std::size_t l) {
            DSTORAGE_REQUEST request;
            ::ZeroMemory(&request, sizeof(request));
            request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
            request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_MEMORY;
            request.Source.File.Source = this->_dstorage_file.get();
            request.Source.File.Offset = o;
            request.Source.File.Size = l;
            request.Destination.Memory.Buffer = d;
            request.Destination.Memory.Size = l;

            this->_dstorage_queue->EnqueueRequest(&request);
            const auto fence_value = ++this->_dstorage_fence_value;
            this->_dstorage_fence->SetEventOnCompletion(fence_value,
                this->_dstorage_event.get());
            this->_dstorage_queue->EnqueueSignal(this->_dstorage_fence.get(),
                fence_value);
            this->_dstorage_queue->Submit();

            ::WaitForSingleObject(this->_dstorage_event.get(), INFINITE);
        }, device, config, power_collector, changed);

#else /* defined(TRROJAN_WITH_DSTORAGE) */
        throw std::logic_error("This build of TRRojan does not support "
            "DirectStorage.");
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

    } else if (trrojan::iequals(method, streaming_method_dstorage_memcpy)) {
#if defined(TRROJAN_WITH_DSTORAGE)
        // Lazily create the event for waiting on the I/O to complete.
        if (!this->_dstorage_event) {
            this->_dstorage_event.attach(::CreateEvent(nullptr, FALSE, FALSE,
                nullptr));
        }
        if (!this->_dstorage_event) {
            throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
        }

        // Lazily create the fence to wait for the operation to complete.
        if (!this->_dstorage_fence) {
            this->_dstorage_fence_value = 0;
            auto hr = device.d3d_device()->CreateFence(
                this->_dstorage_fence_value, D3D12_FENCE_FLAG_NONE,
                IID_PPV_ARGS(this->_dstorage_fence.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        {
            dstorage_configuration c(config);

            // Lazily create the factory.
            if (!this->_dstorage_factory) {
                this->_dstorage_factory = c.create_factory();
            }

            // Apply the configuration.
            c.apply(this->_dstorage_factory, changed);
        }

        // Lazily create the queue.
        if (contains(changed, factor_queue_priority)) {
            this->_dstorage_queue = nullptr;
        }

        if (!this->_dstorage_queue) {
            DSTORAGE_QUEUE_DESC desc;
            ::ZeroMemory(&desc, sizeof(desc));
            desc.Capacity = static_cast<UINT16>(this->pipeline_depth());
            if (desc.Capacity > DSTORAGE_MAX_QUEUE_CAPACITY) {
                desc.Capacity = DSTORAGE_MAX_QUEUE_CAPACITY;
            } else if (desc.Capacity < DSTORAGE_MIN_QUEUE_CAPACITY) {
                desc.Capacity = DSTORAGE_MIN_QUEUE_CAPACITY;
            }
            desc.Priority = static_cast<DSTORAGE_PRIORITY>(priority);
            desc.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
            desc.Device = device.d3d_device();

            auto hr = this->_dstorage_factory->CreateQueue(&desc,
                IID_PPV_ARGS(this->_dstorage_queue.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }
        }

        return this->on_run([this, &folder, repeat](const UINT64 size) {
            this->_buffer.resize(size);
            return this->map_temp_file(size, folder, repeat);

        }, [this, repeat](const UINT64 size) {
            // Remove the file mapping as we cannot open DirectStorage if any
            // other file handle is open on the temporary file.
            this->finalise_temp_file(size, repeat);
            assert(this->_file_view == nullptr);
            this->_file_mapping.close();

            const auto path = get_file_path(this->_file.get());
            this->_file.close();

            auto hr = this->_dstorage_factory->OpenFile(path.c_str(),
                IID_PPV_ARGS(this->_dstorage_file.put()));
            if (FAILED(hr)) {
                throw ATL::CAtlException(hr);
            }

        }, [this](void *d, const std::size_t o, const std::size_t l) {
            DSTORAGE_REQUEST request;
            ::ZeroMemory(&request, sizeof(request));
            request.Options.SourceType = DSTORAGE_REQUEST_SOURCE_FILE;
            request.Options.DestinationType = DSTORAGE_REQUEST_DESTINATION_MEMORY;
            request.Source.File.Source = this->_dstorage_file.get();
            request.Source.File.Offset = o;
            request.Source.File.Size = l;
            request.Destination.Memory.Buffer = this->_buffer.data();
            request.Destination.Memory.Size = l;

            this->_dstorage_queue->EnqueueRequest(&request);
            const auto fence_value = ++this->_dstorage_fence_value;
            this->_dstorage_fence->SetEventOnCompletion(fence_value,
                this->_dstorage_event.get());
            this->_dstorage_queue->EnqueueSignal(this->_dstorage_fence.get(),
                fence_value);
            this->_dstorage_queue->Submit();

            ::WaitForSingleObject(this->_dstorage_event.get(), INFINITE);

            ::memcpy(d, this->_buffer.data(), l);

        }, device, config, power_collector, changed);

#else /* defined(TRROJAN_WITH_DSTORAGE) */
        throw std::logic_error("This build of TRRojan does not support "
            "DirectStorage.");
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

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


/*
 * trrojan::d3d12::sphere_streaming_benchmark::finalise_temp_file
 */
void trrojan::d3d12::sphere_streaming_benchmark::finalise_temp_file(
        const UINT64 size, const std::uint32_t repeat) {
    // Make sure that the initial mapping for receiving the data is removed as
    // we will leak otherwise.
    on_exit([this](void) {
        ::UnmapViewOfFile(this->_file_view);
        this->_file_view = nullptr;
    });

    // Spread out copies as requested.
    for (std::uint32_t i = 0; i < repeat; ++i) {
        ULARGE_INTEGER offset;
        offset.QuadPart = (i + 1) * size;
        auto dst = map_view_of_file(this->_file_mapping, FILE_MAP_WRITE,
            (i + 1) * size, size);
        ::memcpy(this->_file_view, dst, size);
        ::UnmapViewOfFile(dst);
    }
}


/*
 * trrojan::d3d12::sphere_streaming_benchmark::map_temp_file
 */
void *trrojan::d3d12::sphere_streaming_benchmark::map_temp_file(
    const UINT64 size, const std::string &folder,
    const std::uint32_t repeat) {
    if (this->_file_view != nullptr) {
        ::UnmapViewOfFile(this->_file_view);
        this->_file_view = nullptr;
    }

    this->_file.close();
    this->_file_mapping.close();
#if defined(TRROJAN_WITH_DSTORAGE)
    this->_dstorage_file = nullptr;
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

    this->_path = temp_file::create(folder.c_str(), "tssb");

    trrojan::log::instance().write_line(trrojan::log_level::information,
        "Staging data to \"{0}\" ...", this->_path.get());

#if defined(TRROJAN_FOR_UWP)
    {
        auto p = from_utf8(this->_path);
        this->_file.attach(::CreateFile2(p.c_str(),
            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS,
            nullptr));
    }
#else /* defined(TRROJAN_FOR_UWP) */
    this->_file.attach(::CreateFileA(this->_path.get().c_str(),
        GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
#endif /* defined(TRROJAN_FOR_UWP) */
    if (!this->_file) {
        throw ATL::CAtlException(HRESULT_FROM_WIN32(::GetLastError()));
    }

    this->_file_mapping = create_file_mapping(this->_file, PAGE_READWRITE,
        size * (1 + repeat));

    // Note that we only map the first frame here for the I/O worker, because
    // the overall number of frames could exceed what we can map.
    assert(this->_file_view == nullptr);
    this->_file_view = map_view_of_file(this->_file_mapping,FILE_MAP_WRITE,
        0, size);

    return this->_file_view;
}
