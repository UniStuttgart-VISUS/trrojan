// <copyright file="dstorage_sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(TRROJAN_WITH_DSTORAGE)
#include <dstorage.h>

#include <winrt/base.h>

#include "trrojan/d3d12/sphere_benchmark_base.h"
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
        static const std::string implementation_naive;

        /// <summary>
        /// Specifies the approach being implemented.
        /// </summary>
        static const char *factor_implementation;

        /// <summary>
        /// Specifies the factor that determines the capacity of the
        /// DirectStorage queue.
        /// </summary>
        static const char *factor_queue_depth;

        /// <summary>
        /// Specifies the priority of the queue used to stream the data.
        /// </summary>
        /// <remarks>
        /// This is a numeric factor that receives the values of the
        /// <see cref="DSTORAGE_PRIORITY" /> enumeration. If not specified,
        /// the default value is <see cref="DSTORAGE_PRIORITY_NORMAL" />.
        /// </remarks>
        static const char *factor_queue_priority;

        /// <summary>
        /// Determines the directory where the input data are staged.
        /// </summary>
        /// <remarks>
        /// The benchmark will copy the raw data to this directory before
        /// streaming from there, which allows us to test different kinds of
        /// disks.
        /// </remarks>
        static const char *factor_staging_directory;

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

        trrojan::result run_batches(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        trrojan::result run_naive(d3d12::device& device,
            const configuration& config,
            power_collector::pointer& power_collector,
            const std::vector<std::string>& changed);

        winrt::com_ptr<ID3D12Fence> _fence;
        UINT64 _next_fence_value;
        std::wstring _path;
        sphere_streaming_context _stream;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */

#endif /* defined(TRROJAN_WITH_DSTORAGE) */
