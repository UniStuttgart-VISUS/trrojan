// <copyright file="sphere_streaming_context.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
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
        /// Adds the defatult configurations for streaming to the given
        /// configuration set.
        /// </summary>
        /// <param name="configs">The default configuration set to add the
        /// factors controlling streaming to.</param>
        static void add_defaults(trrojan::configuration_set& configs);

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
        /// The name of a factor that controls the simulation of multiple frames
        /// from a single frame provided as input.
        /// </summary>
        /// <remarks>
        /// <para>If this value is larger than zero, the application should copy
        /// the input frame this number of times to simulate seeking within a
        /// filewith real frames.</para>
        /// <para>The context object does not actually work with this factor, ie
        /// it is completely independent from it. The factor is only defined
        /// here for reusability in all streaming benchmarks.</para>
        /// </remarks>
        static const char *factor_repeat_frame;

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
        /// Answer the number of batches that run in parallel in the current
        /// configuration.
        /// </summary>
        inline std::size_t batch_count(void) const noexcept {
            return this->_buffers.size();
        }

        /// <summary>
        /// Answer the number of spheres in each batch.
        /// </summary>
        /// <returns>The number of spheres in a batch, which might be more than
        /// there is actually in the last batch.</returns>
        inline std::size_t batch_elements(void) const noexcept {
            return this->_batch_size;
        }

        /// <summary>
        /// Answer the number of spheres in a specific batch.
        /// </summary>
        /// <param name="batch">The (global) index of the batch, which must be
        /// within [0, this->total_batches()[. Use the global index of the batch
        /// here rather than the index within the ring of
        /// [0, <see cref="batch_count" />[, because the latter does not allow
        /// the method to determine whether this is the final batch.</param>
        /// <returns></returns>
        std::size_t batch_elements(const std::size_t batch) const;

        /// <summary>
        /// Answer the number of bytes in each batch.
        /// </summary>
        /// <returns>The allocated size of a batch in bytes, which might be more
        /// than there is actually in the last batch.</returns>
        inline std::size_t batch_size(void) const noexcept {
            return this->_batch_size * this->_stride;
        }

        /// <summary>
        /// Answer the size of spheres in a specific batch in bytes.
        /// </summary>
        /// <param name="batch">The (global) index of the batch, which must be
        /// within [0, this->total_batches()[. Use the global index of the batch
        /// here rather than the index within the ring of
        /// [0, <see cref="batch_count" />[, because the latter does not allow
        /// the method to determine whether this is the final batch.</param>
        /// <returns></returns>
        inline std::size_t batch_size(const std::size_t batch) const {
            return this->batch_elements(batch) * this->_stride;
        }

        /// <summary>
        /// Gets the buffer holding the given batch on the GPU.
        /// </summary>
        /// <param name="batch">The zero-based index of the batch.</param>
        /// <returns>The buffer holding the data of the batch on the GPU. This
        /// pointer should not be cached.</returns>
        inline winrt::com_ptr<ID3D12Resource> buffer(
                const std::size_t batch) const {
            assert(batch < this->_buffers.size());
            return this->_buffers[batch];
        }

        /// <summary>
        /// Answer the pointer to the mapped data buffer for the given batch.
        /// </summary>
        /// <remarks>
        /// The data pointer might be <c>nullptr</c> if the heap backing the
        /// resources is not of type <c>D3D12_HEAP_TYPE_UPLOAD</c>, because only
        /// upload heaps can be persistently mapped.
        /// </remarks>
        /// <param name="batch">The zero-based index of the batch.</param>
        /// <returns>The mapped memory of the given batch. This pointer should
        /// not be cached.</returns>
        inline void *data(const std::size_t batch) {
            assert(batch < this->_buffers.size());
            return this->_data[batch];
        }

        /// <summary>
        /// Gets a descriptor for the start of the given
        /// <paramref name="batch" />.
        /// </summary>
        /// <param name="batch">The batch to get the GPU address for.</param>
        /// <returns>The GPU address for the given batch.</returns>
        D3D12_GPU_VIRTUAL_ADDRESS descriptor(const std::size_t batch) const;

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
        /// Computes the offset of the given batch in bytes.
        /// </summary>
        /// <remarks>
        /// This offset can be used to index into the source buffer or file.
        /// Note that the method will not perform a bounds check, but happily
        /// report offsets beyond the valid range of the data.
        /// </remarks>
        /// <param name="batch">The index of the batch to compute the offset
        /// for, which must be within [0, <see cref="batch_count()" />[ for
        /// indexing into the mapped GPU memory and within
        /// [0, <see cref="total_batches" />[ for indexing into the source
        /// buffer or file.</param>
        /// <returns>The offset of the given batch.</returns>
        inline std::size_t offset(const std::size_t batch) const noexcept {
            return batch * this->_batch_size * this->_stride;
        }

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
        /// <param name="heap_type"></param>
        /// <param name="initial_state"></param>
        void rebuild(ID3D12Device *device,
            const configuration& config,
            const D3D12_HEAP_TYPE heap_type,
            const D3D12_RESOURCE_STATES initial_state);

        /// <summary>
        /// Gets the offset of the <paramref name="frame" />th repeated frame
        /// in bytes, based on the number of total spheres configured in
        /// <see cref="reshape" />.
        /// </summary>
        /// <param name="frame"></param>
        /// <returns></returns>
        inline std::size_t repeated_frame_offset(const std::size_t frame) {
            assert(frame <= this->_repeat_frame);
            return (frame * this->_total_spheres * this->_stride);
        }

        /// <summary>
        /// Gets the number of times the frame should be repeated when the
        /// tempoary input file is being generated.
        /// </summary>
        /// <remarks>
        /// <para>The streaming context does not consider this parameter in its
        /// internal workings, but merely provides helpers for the caller to
        /// simulate this behaviour. The rationale of integrating the factor
        /// here is that it can be easily reused in multiple locations. The
        /// default value is zero, so the benchmark is instructed to not copy
        /// the frame.</para>
        /// </remarks>
        /// <returns>The number of repetitions of the input frame.</returns>
        inline std::size_t repeat_frame(void) const noexcept {
            return this->_repeat_frame;
        }

        /// <summary>
        /// Resets the stall counter in <see cref="next_batch" /> and returns
        /// the value accumulated until now.
        /// </summary>
        /// <returns></returns>
        inline std::size_t reset_stalls(void) noexcept {
            auto retval = this->_cnt_stalls;
            this->_cnt_stalls = 0;
            return retval;
        }

        /// <summary>
        /// Inform the context that the size of the data set has changed.
        /// </summary>
        /// <remarks>
        /// Reshaping the data most likely requires a rebuild. Therefore, you
        /// must reshape before checking whether a rebuild is needed. The
        /// implementation tries to avoid the need to rebuild whenever possible.
        /// </remarks>
        /// <param name="total_spheres">The total number of particles in the
        /// data set.</param>
        /// <param name="stride">The size of one particle in bytes.</param>
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
        /// <para>The last frame might not hold the full number of particles,
        /// but a remainder. The <see cref="batch_size" /> method considers this
        /// fact by adjusting the size accordingly.</para>
        /// <para>The total number of batches is cached, because we need it
        /// frequently during rendering.</para>
        /// </remarks>
        /// <returns>The total number of batches that need to be rendered for a
        /// frame.</returns>
        inline std::size_t total_batches(void) const noexcept {
            return this->_total_batches;
        }

        /// <summary>
        /// Returns the index of a batch that can be used to upload data to the
        /// GPU or <c>std::numeric_limits&lt;std::size_t&gt;::max()</c> if no
        /// slot is free at the moment.
        /// </summary>
        /// <remarks>
        /// <para>The caller must pass the index returned to
        /// <see cref="signal_done" /> or it will never become available again.
        /// Failing to signal the completion of rendering the batch will
        /// therefore cause the system to hang up once all batches are marked
        /// in flight and none is signalled at all.</para>
        /// </remarks>
        /// <returns>The index of the batch that can now be reused or an invalid
        /// index larger than the number of batches if no batch is free.
        /// </returns>
        std::size_t try_next_batch(void);

        sphere_streaming_context& operator =(
            const sphere_streaming_context&) = delete;

    private:

        std::size_t _batch_size;
        std::vector<winrt::com_ptr<ID3D12Resource>> _buffers;
        std::size_t _cnt_stalls;
        std::vector<void *> _data;
        handle<> _event;
        winrt::com_ptr<ID3D12Fence> _fence;
        std::vector<UINT64> _fence_values;
        winrt::com_ptr<ID3D12Heap> _heap;
        std::atomic<UINT64> _next_fence_value;
        std::size_t _ready_count;
        std::size_t _repeat_frame;
        std::size_t _stride;
        std::size_t _total_batches;
        std::size_t _total_spheres;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */
