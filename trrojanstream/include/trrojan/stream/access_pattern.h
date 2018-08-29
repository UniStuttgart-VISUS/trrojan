/// <copyright file="access_pattern.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>

#include "trrojan/enum_dispatch_list.h"

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
    /// A traits class for inferring the offset between two threads and the
    /// offset between two steps from the
    /// <see cref="trrojan::stream::access_pattern" /> and the problem size
    /// <tparamref name="P" />.
    /// </summary>
    template<access_pattern A, size_t P> struct access_pattern_traits { };

    template<size_t P>
    struct access_pattern_traits<access_pattern::contiguous, P> {
        static inline size_t offset(const size_t rank) {
            return (rank * P);
        }
        static inline size_t step(const size_t parallelism) {
            return 1;
        }
        static inline const std::string& name(void) {
            static const std::string name("contiguous");
            return name;
        }
    };

    template<size_t P>
    struct access_pattern_traits<access_pattern::interleaved, P> {
        static inline size_t offset(const size_t rank) {
            return rank;
        }
        static inline size_t step(const size_t parallelism) {
            return parallelism;
        }
        static inline const std::string& name(void) {
            static const std::string name("interleaved");
            return name;
        }
    };


    template<access_pattern... V>
    using access_pattern_list_t = enum_dispatch_list<access_pattern, V...>;

    typedef access_pattern_list_t<access_pattern::contiguous,
        access_pattern::interleaved> access_pattern_list;
}
}
