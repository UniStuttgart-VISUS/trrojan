// <copyright file="ram_streaming_sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/streaming_sphere_benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Streams spheres from an in-memory buffer.
    /// </summary>
    class TRROJAND3D12_API ram_streaming_sphere_benchmark final
            : public streaming_sphere_benchmark_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        ram_streaming_sphere_benchmark(void);

    protected:

        /// <inheritdoc />
        UINT count_descriptor_tables(const shader_id_type shader_code,
            const bool include_root) const override;

        /// <inheritdoc />
        std::size_t on_batch_required(const std::size_t next_batch,
            const std::size_t batch_size, void *stream_chunk) override;

        /// <inheritdoc />
        void on_data_changed(d3d12::device& device,
            const shader_id_type shader_code,
            const configuration& config) override;

    private:

        std::vector<std::uint8_t> _buffer;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
