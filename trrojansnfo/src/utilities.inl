/// <copyright file="utilities.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


/*
 * trrojan::sysinfo::detail::clone_string
 */
template<class C> C *trrojan::sysinfo::detail::clone_string(const C *src) {
    C *retval = nullptr;

    if (src != nullptr) {
        auto len = std::char_traits<C>::length(src) + 1;
        retval = new C[len];
        ::memcpy(retval, src, len * sizeof(C));
    }

    return retval;
}


/*
 * trrojan::sysinfo::detail::clone_string
 */
template<class C> void trrojan::sysinfo::detail::clone_string(C *&dst,
        const C *src) {
    if (dst != nullptr) {
        delete[] dst;
    }

    dst = clone_string(src);
}


/*
 * trrojan::sysinfo::detail::return_string
 */
template<class C>
bool trrojan::sysinfo::detail::return_string(C *dst, size_t& cntDst,
        const std::basic_string<C>& src) {
    auto req = src.length() + 1;
    auto retval = ((dst != nullptr) && (cntDst >= req));

    // Unconditionally return what we need.
    cntDst = req;

    // Return data if the buffer is big enough.
    if (retval) {
        ::memcpy(dst, src.data(), cntDst * sizeof(C));
    }

    return retval;
}


/*
 * trrojan::sysinfo::detail::string_lookup
 */
template<class C, class I, size_t N>
bool trrojan::sysinfo::detail::string_lookup(C *dst, size_t& cntDst,
        const C *(&src)[N], const I idx) {
    auto retval = (idx < N);

    if (retval) {
        auto res = src[idx];
        auto req = std::char_traits<C>::length(res) + 1;
        retval = ((dst != nullptr) && (cntDst >= req));

        // Unconditionally return what we need.
        cntDst = req;

        // Return data if the buffer is big enough.
        if (retval) {
            ::memcpy(dst, src, cntDst * sizeof(C));
        }

    } else {
        // This is an invalid index, we need no space.
        cntDst = 0;
    }

    return retval;
}
