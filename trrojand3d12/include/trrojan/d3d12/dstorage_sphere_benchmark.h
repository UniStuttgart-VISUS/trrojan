// <copyright file="dstorage_sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include <dstorage.h>

#include <winrt/base.h>

#include "trrojan/temp_file.h"

#include "trrojan/d3d12/sphere_benchmark_base.h"
#include "trrojan/d3d12/sphere_rendering_configuration.h"
#include "trrojan/d3d12/sphere_streaming_context.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Streams spheres using DirectStorage.
    /// </summary>
    class TRROJAND3D12_API dstorage_sphere_benchmark final
            : public sphere_benchmark_base {

    public:

        static const std::string implementation_batches;
        static const std::string implementation_gdeflate;
        static const std::string implementation_naive;

        /// <summary>
        /// Specifies the approach being implemented.
        /// </summary>
        static const char *factor_implementation;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        dstorage_sphere_benchmark(void);

        /// <inheritdoc />
        void optimise_order(configuration_set& configs) override;

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
        /// Sets the vertext buffer for the specified technique and batch.
        /// </summary>
        void set_vertex_buffer(ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code, const std::size_t batch);

    private:

        static result make_result(const configuration& config);

        inline UINT64 enqueue_request(winrt::com_ptr<IDStorageQueue> queue,
                DSTORAGE_REQUEST& request) noexcept {
            queue->EnqueueRequest(&request);
            const auto retval = ++this->_next_fence_value;
            queue->EnqueueSignal(this->_fence.get(), retval);
            //trrojan::log::instance().write_line(log_level::debug,
            //    "Signalling DirectStorage completion with {0}.", retval);
            return retval;
        }

        inline UINT64 enqueue_gdeflate_request(
                winrt::com_ptr<IDStorageQueue> queue,
                DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch,
                const std::size_t slot) noexcept {
            this->make_gdeflate_request(request, frame, batch, slot);
            auto retval = this->enqueue_request(queue, request);
            return retval;
        }

        inline UINT64 enqueue_request(winrt::com_ptr<IDStorageQueue> queue,
                DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch,
                const std::size_t slot) noexcept {
            this->make_request(request, frame, batch, slot);
            auto retval = this->enqueue_request(queue, request);
            return retval;
        }

        void make_gdeflate_request(DSTORAGE_REQUEST& request,
            const std::size_t frame,
            const std::size_t batch) const noexcept;

        inline winrt::com_ptr<ID3D12Resource> make_gdeflate_request(
                DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch,
                const std::size_t slot) const noexcept {
            this->make_gdeflate_request(request, frame, batch);
            auto retval = this->_stream.buffer(slot);
            request.Destination.Buffer.Resource = retval.get();
            return retval;
        }

        inline void make_request(DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch) const noexcept {
            request.Source.File.Offset = this->_stream.offset(batch);
            request.Source.File.Size = this->_stream.batch_size(batch);
            request.Destination.Buffer.Offset = 0;
            request.Destination.Buffer.Size = request.Source.File.Size;
        }

        inline winrt::com_ptr<ID3D12Resource> make_request(
                DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch,
                const std::size_t slot) const noexcept {
            this->make_request(request, frame, batch);
            auto retval = this->_stream.buffer(slot);
            request.Destination.Buffer.Resource = retval.get();
            return retval;
        }

        trrojan::result run_batches(d3d12::device& device,
            const configuration& config,
            const sphere_rendering_configuration& sphere_config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        trrojan::result run_gdeflate(d3d12::device& device,
            const configuration& config,
            const sphere_rendering_configuration& sphere_config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        trrojan::result run_naive(d3d12::device& device,
            const configuration& config,
            const sphere_rendering_configuration& sphere_config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        inline UINT64 submit_request(winrt::com_ptr<IDStorageQueue> queue,
                DSTORAGE_REQUEST& request,
                const std::size_t frame,
                const std::size_t batch,
                const std::size_t slot) noexcept {
            this->make_request(request, frame, batch, slot);
            auto retval = this->enqueue_request(queue, request);
            queue->Submit();
            return retval;
        }

        std::vector<std::size_t> _batches;
        std::vector<std::uint8_t> _buffer;
        winrt::com_ptr<ID3D12Fence> _fence;
        UINT64 _next_fence_value;
        temp_file _path;
        sphere_streaming_context _stream;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */

#endif /* defined(TRROJAN_WITH_DSTORAGE) */
