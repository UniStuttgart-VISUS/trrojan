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

        sphere_streaming_context(const configuration& config);

        /// <summary>
        /// Move <paramref name="rhs" /> into a new instance.
        /// </summary>
        sphere_streaming_context(sphere_streaming_context&& rhs);

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
        /// Answer the total number of batches that need to be rendered for
        /// streaming a whole frame given the current configuration.
        /// </summary>
        inline std::size_t total_batches(void) const {
            return this->_total_batches;
        }

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs"></param>
        /// <returns></returns>
        sphere_streaming_context& operator =(sphere_streaming_context&& rhs);

    private:

        void alloc_buffer(ID3D12Device *device, const std::size_t stride,
            const std::size_t pipeline_depth);

        std::size_t _batch_count;
        std::size_t _batch_size;
        ATL::CComPtr<ID3D12Resource> _buffer;
        void *_data;
        std::size_t _total_batches;
    };

} /* end namespace d3d11 */
} /* end namespace trrojan */
