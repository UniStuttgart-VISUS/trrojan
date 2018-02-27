/// <copyright file="utilities.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include <cinttypes>
#include <memory>
#include <string>


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// Return <paramref name="src" /> to <paramref name="dst" />, but only if
    /// <paramref name="cntDst" /> indicates a sufficiently large buffer.
    /// </summary>
    template<class C>
    bool return_string(C *dst, size_t& cntDst, const std::basic_string<C>& src);

    /// <summary>
    /// Answer the number of elements in a stack-allocated array.
    /// </summary>
    /// <remarks>
    /// This is a backport of <see cref="std::size" /> from the C++17 STL, which
    /// is already available in VS2015, but not in gcc.
    /// </remarks>
    template<class T, size_t S>
    inline constexpr size_t size(const T(&)[S]) noexcept {
        return S;
    }

    /// <summary>
    /// Performs lookup of the <paramref name="idx" />th item in the string
    /// table <paramref name="src" />.
    /// </summary>
    template<class C, class I, size_t N>
    bool string_lookup(C *dst, size_t& cntDst, const C *(&src)[N], const I idx);

} /* end detail */
} /* end sysinfo */
} /* end trrojan */

#include "utilities.inl"
