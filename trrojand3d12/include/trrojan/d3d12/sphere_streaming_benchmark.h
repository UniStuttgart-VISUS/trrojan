// <copyright file="sphere_streaming_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include <dstorage.h>
#endif /* defined(TRROJAN_WITH_DSTORAGE) */

#include <winrt/base.h>

#include "trrojan/memory_unmapper.h"
#include "trrojan/temp_file.h"

#include "trrojan/d3d12/sphere_benchmark_base.h"
#include "trrojan/d3d12/sphere_streaming_context.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Streams spheres directly from disk or RAM.
    /// </summary>
    class TRROJAND3D12_API sphere_streaming_benchmark final
            : public sphere_benchmark_base {

    public:

        /// <summary>
        /// Specifies the priority of the queue used to stream the data in case
        /// DirectStorage is selected as streaming method.
        /// </summary>
        static const char *factor_queue_priority;

        /// <summary>
        /// Determines how the data are streamed to the GPU.
        /// </summary>
        static const char *factor_streaming_method;

        /// <summary>
        /// Identifies the method of streaming from a memory-mapped file that is
        /// re-mapped in batches.
        /// </summary>
        static const std::string streaming_method_batch_memory_mapping;

        /// <summary>
        /// Identifies the method of streaming via DirectStorage into the
        /// memory-mapped buffer.
        /// </summary>
        static const std::string streaming_method_dstorage;

        /// <summary>
        /// Identifies the method of streaming via memory using DirectStorage.
        /// </summary>
        static const std::string streaming_method_dstorage_memcpy;

        /// <summary>
        /// Identifies the method of streaming from disk using an I/O ring.
        /// </summary>
        static const std::string streaming_method_io_ring;

        /// <summary>
        /// Identifies the method of streaming from a memory-mapped file for
        /// which the view is created whenever we want to read.
        /// </summary>
        static const std::string streaming_method_memory_mapping;

        /// <summary>
        /// Identifies the method of streaming from a memory-mapped file that is
        /// mapped as a whole.
        /// </summary>
        static const std::string streaming_method_persistent_memory_mapping;

        /// <summary>
        /// Identifies the method of loading the whole data set to RAM and
        /// streaming from there.
        /// </summary>
        static const std::string streaming_method_ram;

        /// <summary>
        /// Identifies the method of reading directly from disk for streaming.
        /// </summary>
        static const std::string streaming_method_read_file;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        sphere_streaming_benchmark(void);

        /// <summary>
        /// Fonalises the instance.
        /// </summary>
        virtual ~sphere_streaming_benchmark(void);

    protected:

        /// <summary>
        /// Checks whether the stream and any derived GPU resources like the
        /// command lists need to be adjusted to match the given configuration.
        /// </summary>
        bool check_stream_changed(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed);

        /// <inheritdoc />
        bool clear_stale_data(const std::vector<std::string>& changed) override;

        /// <inheritdoc />
        UINT count_descriptor_tables(const shader_id_type shader_code,
            const bool include_root) const override;

        /// <inheritdoc />
        void on_device_switch(device& device) override;

        /// <inheritdoc />
        trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed) override;

        /// <summary>
        /// The actual implementation of the benchmark, which uses
        /// <paramref name="copy_data" /> to obtain the data for each
        /// batch.
        /// </summary>
        template<class TAllocate, class TCleanup, class TCopy>
        trrojan::result on_run(TAllocate&& allocate_data,
            TCleanup&& load_cleanup,
            TCopy&& copy_data,
            d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        /// <summary>
        /// Sets the vertext buffer for the specified technique and batch.
        /// </summary>
        void set_vertex_buffer(ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code, const std::size_t batch);

    private:

        typedef std::unique_ptr<void, memory_unmapper> mapping_type;

        void finalise_temp_file(const UINT64 size, const std::uint32_t repeat);

        std::unique_ptr<void, memory_unmapper>& map_temp_file(const UINT64 size,
            const std::string& folder, const std::uint32_t repeat);

        std::pair<mapping_type, std::size_t> map_view_of_file(
            const DWORD access, const std::size_t offset,
            const std::size_t size);

        void prepare_dstorage(ID3D12Device *device, const configuration& config,
            const std::vector<std::string>& changed);

        DWORD _allocation_granularity;
        std::vector<std::uint8_t> _buffer;
        std::size_t _cnt_remap;
#if defined(TRROJAN_WITH_DSTORAGE)
        winrt::com_ptr<IDStorageFactory> _dstorage_factory;
        winrt::com_ptr<ID3D12Fence> _dstorage_fence;
        UINT64 _dstorage_fence_value;
        winrt::com_ptr<IDStorageFile> _dstorage_file;
        winrt::handle _dstorage_event;
        winrt::com_ptr<IDStorageQueue> _dstorage_queue;
#endif /* defined(TRROJAN_WITH_DSTORAGE) */
        winrt::file_handle _file;
        winrt::handle _file_mapping;
        mapping_type _file_view;
        std::pair<std::size_t, std::size_t> _mapped_range;
        temp_file _path;
        sphere_streaming_context _stream;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/sphere_streaming_benchmark.inl"
