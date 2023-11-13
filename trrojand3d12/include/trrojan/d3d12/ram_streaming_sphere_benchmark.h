// <copyright file="ram_streaming_sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/sphere_benchmark_base.h"
#include "trrojan/d3d12/sphere_streaming_context.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Streams spheres from an in-memory buffer.
    /// </summary>
    class TRROJAND3D12_API ram_streaming_sphere_benchmark final
            : public sphere_benchmark_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        ram_streaming_sphere_benchmark(void);

    protected:

        /// <summary>
        /// Checks whether the stream and any derived GPU resources like the
        /// command lists need to be adjusted to match the given configuration.
        /// </summary>
        /// <param name="device">The device on which the GPU resources should be
        /// allocated.</param>
        /// <param name="config">The configuration that is run next and to which
        /// the stream needs to be adjusted.</param>
        /// <param name"changed">The names of the factors that have changed,
        /// which removes the need for the method to check this by itself.
        /// </param>
        void check_stream_changed(d3d12::device& device,
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
        /// <param name="cmd_list">The command list for which the vertex buffer
        /// should be set.</param>
        /// <param name="shader_code">The shader code, which determines whether
        /// the vertex buffer must be set at all or whether it can be ignored.
        /// </param>
        /// <param name="batch">The index of the batch, which determines the
        /// offset into the vertex buffer at which rendering should start and
        /// how many spheres should be rendered. Note that the buffer size
        /// reported is independent from the batch number, so this will always
        /// be the maximum number of spheres being rendered in the current
        /// configuration.</param>
        void set_vertex_buffer(ID3D12GraphicsCommandList *cmd_list,
            const shader_id_type shader_code, const std::size_t batch);

    private:

        std::vector<std::uint8_t> _buffer;
        sphere_streaming_context _stream;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
