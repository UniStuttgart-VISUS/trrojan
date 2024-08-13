// <copyright file="aligned_allocator.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::aligned_allocator<TType, Alignment>::allocate
 */
template<class TType>
typename trrojan::aligned_allocator<TType>::value_type *
trrojan::aligned_allocator<TType>::allocate(const size_type n) {
    assert(n <= (std::numeric_limits<size_type>::max)() / sizeof(value_type));

#if defined(_MSC_VER)
    auto retval = ::_aligned_malloc(n * sizeof(value_type), this->_alignment);
#else /* defined(_MSC_VER) */
    auto retval = ::aligned_alloc(this->_alignment, n * sizeof(value_type));
#endif /* defined(_MSC_VER) */

    if (retval == nullptr) {
        throw std::bad_alloc();
    }

    return static_cast<value_type *>(retval);
}


/*
 * trrojan::aligned_allocator<TType>::deallocate
 */
template<class TType>
void trrojan::aligned_allocator<TType>::deallocate(value_type *p,
        const size_type n) noexcept {
    if (p != nullptr) {
#if defined(_MSC_VER)
        ::_aligned_free(p);
#else /* defined(_MSC_VER) */
        std::free(p);
#endif /* defined(_MSC_VER) */
    }
}


/*
 * trrojan::aligned_allocator<TType>::operator =
 */
template<class TType>
template<class T>
trrojan::aligned_allocator<TType>&
trrojan::aligned_allocator<TType>::operator =(const aligned_allocator<T>& rhs) {
    this->_alignment = rhs._alignment;
    return *this;
}
