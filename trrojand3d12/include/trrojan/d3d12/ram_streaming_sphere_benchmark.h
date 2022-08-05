// <copyright file="ram_streaming_sphere_benchmark.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
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

        /// <inheritdoc />
        UINT count_descriptor_tables(const shader_id_type shader_code,
            const bool include_root) const override;

        /// <inheritdoc />
        trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) override;

    private:

        std::vector<std::uint8_t> _buffer;
        sphere_streaming_context _stream;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
