// <copyright file="aligned_allocator.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::aligned_allocator<TType, Alignment>::allocate
 */
template<class TType, std::size_t Alignment>
typename trrojan::aligned_allocator<TType, Alignment>::pointer
trrojan::aligned_allocator<TType, Alignment>::allocate(const size_type n) {
    assert(n <= (std::numeric_limits<size_type>::max)() / sizeof(value_type));

#if defined(_MSC_VER)
    auto retval = ::_aligned_alloc(Alignment, n * sizeof(value_type));
#else /* defined(_MSC_VER) */
    auto retval = ::aligned_alloc(Alignment, n * sizeof(value_type));
#endif /* defined(_MSC_VER) */

    if (retval == nullptr) {
        throw std::bad_alloc();
    }

    return static_cast<value_type *>(retval);
}


/*
 * trrojan::aligned_allocator<TType, Alignment>::deallocate
 */
template<class TType, std::size_t Alignment>
void trrojan::aligned_allocator<TType, Alignment>::deallocate(pointer p,
        const size_type n) noexcept {
    if (p != nullptr) {
#if defined(_MSC_VER)
        ::_aligned_free(p);
#else /* defined(_MSC_VER) */
        std::free(p);
#endif /* defined(_MSC_VER) */
    }
}
