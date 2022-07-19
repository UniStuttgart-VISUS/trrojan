// <copyright file="aligned_allocator.h" company="Visualisierungsinstitut der Universität Stuttgart">
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
    /// An allocator that ensures alignment of the allocations.
    /// </summary>
    /// <typeparam name="TType"></typeparam>
    template<class TType> class aligned_allocator {

    public:

        /// <summary>
        /// The type used to express counters and memory size.
        /// </summary>
        typedef std::size_t size_type;

        /// <summary>
        /// The type determining the size of one object allocated by this
        /// allocator.
        /// </summary>
        typedef TType value_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="alignment"></param>
        aligned_allocator(const size_type alignment = 0) noexcept
            : _alignment(alignment) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="rhs"></param>
        /// <returns></returns>
        template<class T>
        aligned_allocator(const aligned_allocator<T> &rhs) noexcept
            : _alignment(rhs._alignment) { }

        /// <summary>
        /// Allocates aligned memory for <paramref name="n" /> elements of
        /// <see cref="value_type" />.
        /// </summary>
        /// <param name="n"></param>
        /// <returns></returns>
        value_type *allocate(const size_type n);

        /// <summary>
        /// Frees an allocation created by this allocator.
        /// </summary>
        /// <param name="p"></param>
        /// <param name="n"></param>
        /// <returns></returns>
        void deallocate(value_type *p, const size_type n) noexcept;

        /// <summary>
        /// Assignment
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="rhs"></param>
        /// <returns></returns>
        template<class T>
        aligned_allocator& operator =(const aligned_allocator<T> &rhs);

    private:

        size_type _alignment;

        // Allow siblings copy the alignment.
        template<class T> friend class aligned_allocator;
    };

} /* end namespace trrojan */

#include "trrojan/aligned_allocator.inl"
