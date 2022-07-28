// <copyright file="streaming_sphere_benchmark_base.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <atomic>

#include "trrojan/d3d12/sphere_benchmark_base.h"


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// Base class for benchmarks streaming spheres to the GPU.
    /// </summary>
    class TRROJAND3D12_API streaming_sphere_benchmark_base
            : public sphere_benchmark_base {

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
        /// Finalise the instance.
        /// </summary>
        virtual ~streaming_sphere_benchmark_base(void) = default;

    protected:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        streaming_sphere_benchmark_base(const std::string& name);

        /// <summary>
        /// Gets the pointer to the <paramref name="batch_index" />th batch
        /// of the <paramref name="frame_index" />th frame in the streaming
        /// buffer.
        /// </summary>
        /// <param name="batch_index"></param>
        /// <param name="frame_index"></param>
        /// <returns></returns>
        void *get_stream_chunk(const std::size_t batch_index,
            const std::size_t frame_index);

        /// <summary>
        /// Reacts to the batch size changing by allocating the appropriate
        /// buffer and mapping it to <see cref="_stream" />.
        /// </summary>
        /// <remarks>
        /// Subclasses overriding this method must call the parent class
        /// implementation to make sure that all book keeping on the state of
        /// the batches is correct.
        /// </remarks>
        /// <param name="device"></param>
        /// <param name="count"></param>
        /// <param name="size"></param>
        virtual void on_batch_changed(device& device, const std::size_t count,
            const std::size_t size);

        /// <summary>
        /// Gets called by the base class once a batch was rendered and a new
        /// one can be scheduled.
        /// </summary>
        /// <param name="next_batch">The index of the next batch that the method
        /// should deliver to <paramref name="stream_chunk" />. The base class
        /// will make sure that this method is only called if this is a valid
        /// batch for the currently loaded data set. Subclasses do not need to
        /// perform this test again.</param>
        /// <param name="batch_size">The size (in number of particles) of a
        /// batch to load. This might be more than the remaining data in case
        /// of the last batch. Subclasses must determine the actual number of
        /// remaining data available.</param>
        /// <param name="stream_chunk">The chunk in the persistently mapped
        /// streaming buffere where the data should be copied to.</param>
        /// <returns>The actual batch size, ie the number of spheres copied to
        /// <paramref name="stream_chunk" />.</returns>
        virtual std::size_t on_batch_required(const std::size_t next_batch,
            const std::size_t batch_size, void *stream_chunk) = 0;

        /// <summary>
        /// Prepares in-memory data (if necessary) and the upload heap for
        /// streaming parts of the data.
        /// </summary>
        virtual void on_data_changed(d3d12::device& device,
            const shader_id_type shader_code, const configuration& config) = 0;

        /// <inheritdoc />
        void on_device_switch(device& device) override;

        /// <inheritdoc />
        trrojan::result on_run(d3d12::device& device,
            const configuration& config,
            const std::vector<std::string>& changed) override;

    private:

        /// <summary>
        /// The context information passed to the timer callback
        /// <see cref="event_callback" /> in order to dispatch the event to
        /// the correct benchmark class and chunk index.
        /// </summary>
        struct event_callback_context {
            streaming_sphere_benchmark_base *benchmark;
            std::size_t index;
        };

        /// <summary>
        /// A callback scheduled using
        /// <see cref="RegisterWaitForSingleObject" /> to wait for a fence
        /// becoming signalled.
        /// </summary>
        /// <remarks>
        /// <para>This method unpacks the context and calls
        /// <see cref="on_batch_required" /> to request new data if there are
        /// data left to load.</para>
        /// </remarks>
        /// <param name="context"></param>
        /// <param name="timeout"></param>
        static void CALLBACK event_callback(void *context, BOOLEAN timeout);

        std::size_t _batch_count;
        std::size_t _batch_size;
        std::vector<event_callback_context> _contexts;
        std::vector<handle<>> _events;
        ATL::CComPtr<ID3D12Fence> _fence;
        std::atomic<std::size_t> _index_draw;
        std::atomic<std::size_t> _index_load;
        void *_stream;
        std::size_t _total_batches;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
