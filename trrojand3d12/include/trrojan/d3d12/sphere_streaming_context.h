// <copyright file="sphere_streaming_context.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <memory>
#include <cstddef>
#include <type_traits>
#include <vector>

#include <Windows.h>

#include <winrt/base.h>

#include "trrojan/d3d12/handle.h"
#include "trrojan/d3d12/sphere_benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// This streaming context manages splitting particle data sets into
    /// multiple batches that are streamed to the GPU one after the other.
    /// </summary>
    /// <remarks>
    /// <para>The context manages the factors controlling batch sizes and
    /// computes the number of required batches for the data set. It therefore
    /// needs to be updated if any of the streaming parameters, but also the
    /// data set itself change.</para>
    /// <para>The context also manages the GPU memory used for streaming, which
    /// a persistently mapped buffer that can be filled by means of
    /// <see cref="memcpy" />. The context splits this buffer into the requested
    /// number of batches and tracks which of them are currently used for
    /// uploading and which of them are currently rendered. In order for the
    /// latter to work, callers must invoke <see cref="signal_done" /> in the
    /// command queue used for rendering after each batch has been submitted to
    /// the GPU. This allows the context to potentially wait in case all of the
    /// batches are being used by the renderer while at the same time, the I/O
    /// thread requests a new buffer for upload.</para>
    /// <para>The context does not track asynchronous loading operations, eg
    /// requests submitted to DirectStorage. The application is responsible for
    /// doing that by not submitting batches that are not ready for rendering.
    /// </para>
    /// </remarks>
    class TRROJAND3D12_API sphere_streaming_context final {

    public:

        /// <summary>
        /// The name of the factor &quot;batch_count&quot;
        /// </summary>
        /// <remarks>
        /// <para>The batch size is a <c>unsigned int</c> factor that determines
        /// how many batches are used in parallel on the GPU, ie the batch size
        /// is the size of the ring buffer on the GPU.</para>
        /// <para>Note that the context does not care about the depth of the
        /// pipeline, so the ring buffer might wrap around frame borders. It is
        /// the duty of the application to allocate enough batches to support
        /// the number of in-flight frame it creates.</para>
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
        /// Finalises the instance.
        /// </summary>
        ~sphere_streaming_context(void);

        /// <summary>
        /// Gets the offset and length (in bytes) of the
        /// <paramref name="batch" />th batch.
        /// </summary>
        /// <param name="batch"></param>
        /// <param name="frame"></param>
        /// <returns>The offset and size, both in bytes, of the batch.</returns>
        std::pair<std::size_t, std::size_t> batch(
            const std::size_t batch) const;

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
        /// Gets the start of the <paramref name="batch" />th batch in the
        /// persistently mapped buffer, ie the copy target for this batch.
        /// </summary>
        /// <param name="batch"></param>
        /// <returns></returns>
        inline void *data(const std::size_t batch) {
            return static_cast<std::uint8_t *>(this->_data)
                + this->batch(batch).first;
        }

        /// <summary>
        /// Gets a descriptor for the start of the data buffer.
        /// </summary>
        /// <param name=""></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS descriptor(void) const;

        /// <summary>
        /// Gets a descriptor for the start of the given
        /// <paramref name="batch" />.
        /// </summary>
        /// <param name="batch"></param>
        /// <returns></returns>
        D3D12_GPU_VIRTUAL_ADDRESS descriptor(const std::size_t batch) const;

        /// <summary>
        /// The total size of a single frame of the current configuration in
        ///  bytes.
        /// </summary>
        /// <returns>The size of a frame in bytes.</returns>
        std::size_t frame_size(void) const noexcept;

        /// <summary>
        /// Answer the number of particles in the last batch, which might be
        /// less than <see cref="batch_size" /> if the total number of particles
        /// is not divisble by the requested batch size.
        /// </summary>
        /// <returns>The number of particles in the last batch.</returns>
        std::size_t last_batch_size(void) const noexcept;

        /// <summary>
        /// Returns the index of a batch that can be used to upload data to the
        /// GPU.
        /// </summary>
        /// <remarks>
        /// <para>The method may block waiting for a fence injected into the
        /// command queue via <see cref="signal_done" /> if all batches are
        /// marked as in flight.</para>
        /// <para>The caller must pass the index returned to
        /// <see cref="signal_done" /> or it will never become available again.
        /// Failing to signal the completion of rendering the batch will
        /// therefore cause the system to hang up once all batches are marked
        /// in flight and none is signalled at all.</para>
        /// </remarks>
        /// <returns>The index of the batch that can now be reused.</returns>
        std::size_t next_batch(void);

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
        void rebuild(ID3D12Device *device, const configuration& config);

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
        /// Signal that the GPU is done with rendering the
        /// <paramref name="batch"/>th batch in the given command
        /// <paramref name="queue" />.
        /// </summary>
        /// <remarks>
        /// <para>This method injects a fence into the command queue. Once the
        /// GPU is past that fence, the <see cref="next_batch" /> method is free
        /// to return <paramref name="batch" /> for reuse.</para>
        /// </remarks>
        /// <param name="batch"></param>
        /// <param name="queue"></param>
        void signal_done(const std::size_t batch, ID3D12CommandQueue *queue);

        /// <summary>
        /// Answer the total number of batches that need to be rendered for
        /// streaming a whole frame given the current configuration (number of
        /// total spheres and batch size).
        /// </summary>
        /// <remarks>
        /// The last frame might not hold the full <see cref="batch_size" /> of
        /// particles.
        /// </remarks>
        /// <returns>The total number of batches that need to be rendered for a
        /// frame.</returns>
        std::size_t total_batches(void) const;

        sphere_streaming_context& operator =(
            const sphere_streaming_context&) = delete;

    private:

        std::size_t _batch_count;
        std::size_t _batch_size;
        winrt::com_ptr<ID3D12Resource> _buffer;
        void *_data;
        handle<> _event;
        winrt::com_ptr<ID3D12Fence> _fence;
        std::vector<UINT64> _fence_values;
        UINT64 _next_fence_value;
        std::size_t _ready_count;
        std::size_t _stride;
        std::size_t _total_spheres;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
