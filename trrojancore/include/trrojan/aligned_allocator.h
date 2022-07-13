// <copyright file="aligned_allocator" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>
#include <stdexcept>

#include <malloc.h>


namespace trrojan {

    /// <summary>
    /// An allocator that ensures alignment of the allocator.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    template<class TType, std::size_t Alignment>
    struct aligned_allocator {

        typedef const TType *const_pointer;
        typedef const TType& const_reference;
        typedef typename std::allocator<TType>::difference_type difference_type;
        typedef TType *pointer;
        typedef TType& reference;
        typedef TType value_type;
        typedef std::size_t size_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        aligned_allocator(void) = default;

        /// <summary>
        /// Allocates aligned memory for <paramref name="n" /> elements of
        /// <see cref="value_type" />.
        /// </summary>
        /// <param name="n"></param>
        /// <returns></returns>
        pointer allocate(const size_type n);

        /// <summary>
        /// Frees an allocation.
        /// </summary>
        /// <param name="p"></param>
        /// <param name="n"></param>
        /// <returns></returns>
        void deallocate(pointer p, const size_type n) noexcept;
    };

} /* end namespace trrojan */

#include "trrojan/aligned_allocator.inl"
