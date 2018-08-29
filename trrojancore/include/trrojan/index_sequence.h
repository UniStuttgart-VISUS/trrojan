/// <copyright file="index_sequence.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/export.h"

#include <cstdint>
#include <type_traits>
#include <vector>


namespace trrojan {

    /// <summary>
    /// A compile-time sequence of integers.
    /// </summary>
    template<class T, T... V> struct integer_sequence {
        static_assert(std::is_integral<T>::value, "The value type of an "
            "integer_sequence must be an integral type.");

        /// <summary>
        /// The type of the sequence itself.
        /// </summary>
        typedef integer_sequence<T, V...> type;

        /// <summary>
        /// The type of a single integer.
        /// </summary>
        typedef T value_type;

        /// <summary>
        /// Answer the number of elements in <tparamref name="V" />.
        /// </summary>
        static constexpr size_t size(void) noexcept {
            return (sizeof...(V));
        }

        /// <summary>
        /// Answer the elements of the sequence.
        /// </summary>
        static constexpr std::vector<T> to_vector(void) {
            return { V... };
        }
    };

namespace detail {
    template<bool N, bool Z, class V, class S> struct make_integer_sequence {
        static_assert(!N, "The size of an integer_sequence must be positive.");
    };

    template<class T, T V, T... Vs>
    struct make_integer_sequence<false, false, std::integral_constant<T, V>, integer_sequence<T, Vs...> >
        : make_integer_sequence<false, (V == 1), std::integral_constant<T, V - 1>, integer_sequence<T, V - 1, Vs...>> { };

    template<class T, T... Vs> struct make_integer_sequence<false, true, std::integral_constant<T, 0>, integer_sequence<T, Vs...>>
        : integer_sequence<T, Vs...> { };
}

    /// <summary>
    /// A compile-time sequence of <tparamref name="N" /> integral values of
    /// type <tparamref name="T" /> starting from zero.
    /// </summary>
    template<class T, T N>
    using make_integer_sequence = typename detail::make_integer_sequence<
        (N < 0),
        (N == 0),
        std::integral_constant<T, N>,
        integer_sequence<T>>::type;

    /// <summary>
    /// A compile-time sequence of <tparamref name="N" /> integral values
    /// starting from zero.
    /// </summary>
    template<size_t N>
    using index_sequence = make_integer_sequence<size_t, N>;
}
