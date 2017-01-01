/// <copyright file="access_pattern.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/stream/export.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// Possible memory access patterns when using multiple threads.
    /// </summary>
    enum class TRROJANSTREAM_API access_pattern {

        /// <summary>
        /// This access pattern instructs each thread to access a continguous
        /// range of memory, ie the threads read from a potentially widespread
        /// memory range at the same time.
        /// </summary>
        contiguous,

        /// <summary>
        /// This access pattern instruct the threads to access the memory in an
        /// interleaved manner, ie all threads read in a spatially close range
        /// at approximately the same time.
        /// </summary>
        interleaved
    };


    /// <summary>
    /// A traits class for inferring the offset between two steps from the
    /// <see cref="trrojan::stream::access_pattern" /> and the problem size.
    /// </summary>
    template<access_pattern A, size_t P> struct access_pattern_traits { };

    template<size_t P>
    struct access_pattern_traits<access_pattern::contiguous, P> {
        static const size_t offset = 1;
    };

    template<size_t P>
    struct access_pattern_traits<access_pattern::interleaved, P> {
        static const size_t offset = P;
    };
}
}
