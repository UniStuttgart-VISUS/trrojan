// <copyright file="sphere_streaming_context.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/d3d12/sphere_benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// 
    /// </summary>
    class TRROJAND3D12_API sphere_streaming_context final {

    public:

        /// <summary>
        /// The name of the factor &quot;batch_count&quot;
        /// </summary>
        /// <remarks>
        /// The batch size is a <c>unsigned int</c> factor that determines how
        /// many batches are used in parallel per frame.
        /// </remarks>
        static const char *factor_batch_count;

        /// <summary>
        /// The name of the factor &quot;batch_size&quot;
        /// </summary>
        /// <remarks>
        /// The batch size is a <c>unsigned int</c> factor that determines how
        /// many particles are rendered at once.
        /// </remarks>
        static const char *factor_batch_size;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        sphere_streaming_context(void);

        sphere_streaming_context(const sphere_streaming_context&) = delete;

        /// <summary>
        /// Gets the offset and length (in bytes) of the
        /// <paramref name="batch" />th batch of the
        /// <paramref name="frame" />th frame.
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="frame"></param>
        /// <returns>The offset and size, both in bytes, of the batch.</returns>
        std::pair<std::size_t, std::size_t> batch(const std::size_t batch,
            const std::size_t frame) const;

        /// <summary>
        /// Answer the number of batches that run in parallel in the current
        /// configuration.
        /// </summary>
        inline std::size_t batch_count(void) const {
            return this->_batch_count;
        }

        /// <summary>
        /// Answer the number of spheres in each batch.
        /// </summary>
        inline std::size_t batch_size(void) const {
            return this->_batch_size;
        }

        /// <summary>
        /// Answer the pointer to the mapped data buffer.
        /// </summary>
        inline void *data(void) noexcept {
            return this->_data;
        }

        /// <summary>
        /// Gets the start of the <paramref name="batch" />th batch of the
        /// <paramref name="frame" />th frame.
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="frame"></param>
        /// <returns></returns>
        inline void *data(const std::size_t batch, const std::size_t frame) {
            return static_cast<std::uint8_t *>(this->_data)
                + this->batch(batch, frame).first;
        }

        /// <summary>
        /// Gets a descriptor for the start of the data buffer.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS descriptor(void) const;

        /// <summary>
        /// Gets a descriptor for the start of the given
        /// <paramref name="batch" /> of the given <paramref name="frame" />.
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="frame"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS descriptor(const std::size_t batch,
            const std::size_t frame) const;

        /// <summary>
        /// Answer the size in bytes of all batches of a single frame.
        /// </summary>
        std::size_t frame_size(void) const;

        /// <summary>
        /// Answer whether changing any of the given factors requires a rebuild
        /// of the context's resources.
        /// </summary>
        bool rebuild_required(const std::vector<std::string>& changed) const;

        /// <summary>
        /// Rebuilds the streaming buffer on the given device.
        /// </summary>
        /// <param name="device"></param>
        /// <param name="config"></param>
        /// <param name="pipeline_depth"></param>
        void rebuild(ID3D12Device *device, const configuration& config,
            const std::size_t pipeline_depth);

        /// <summary>
        /// Inform the context that the size of the data set has changed.
        /// </summary>
        /// <remarks>
        /// Reshaping the data most likely requires a rebuild. Therefore, you
        /// must reshape before checking whether a rebuild is needed. The
        /// implementation tries to avoid the need to rebuild whenever possible.
        /// </remarks>
        /// <param name="total_spheres"></param>
        /// <param name="stride"></param>
        /// <returns><c>true</c> if the buffer needs to be rebuilt, <c>false</c>
        /// otherwise.</returns>
        bool reshape(const std::size_t total_spheres, const std::size_t stride);

        /// <summary>
        /// Inform the context about new data.
        /// </summary>
        /// <param name="data"></param>
        /// <returns><c>true</c> if the buffer needs to be rebuilt, <c>false</c>
        /// otherwise.</returns>
        bool reshape(const sphere_data& data) {
            return this->reshape(data.spheres(), data.stride());
        }

        /// <summary>
        /// Answer the total number of batches that need to be rendered for
        /// streaming a whole frame given the current configuration.
        /// </summary>
        std::size_t total_batches(void) const;

        sphere_streaming_context& operator =(
            const sphere_streaming_context&) = delete;

    private:

        std::size_t _batch_count;
        std::size_t _batch_size;
        ATL::CComPtr<ID3D12Resource> _buffer;
        void *_data;
        std::size_t _stride;
        std::size_t _total_spheres;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
