/// <copyright file="utilities.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include <cinttypes>
#include <memory>
#include <string>
#include <vector>


namespace trrojan {
namespace sysinfo {
namespace detail {

    /// <summary>
    /// Allocate a copy of <paramref name="src" /> using new[] and return
    /// ownership of the new buffer.
    /// </summary>
    template<class C> C *clone_string(const C *src);

    /// <summary>
    /// Read all contents of a binary file.
    /// </summary>
    std::vector<std::uint8_t> read_all_bytes(const char *path);

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
