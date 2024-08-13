// <copyright file="enum_dispatch_list.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <type_traits>
#include <utility>


namespace trrojan {

    /// <summary>
    /// Template for dispatch lists which allow for defining lists of enum
    /// values.
    /// </summary>
    /// <remarks>
    /// The intended use of this class is the definition of a dispatch list type
    /// for an enumeration type <c>et</c> like this
    /// <c>template<et... V> using et_list_t = enum_dispatch_list<et, V...>;</c>
    /// </remarks>
    /// <tparam name="E">The type of the enumeration.</tparam>
    /// <tparam name="V">The enumeration values.</tparam>
    template<class E, E... V> struct enum_dispatch_list { };


namespace detail {

    /// <summary>
    /// Helper for casting elements of an integer sequence to enumeration
    /// values in a <see cref="enum_dispatch_list" />.
    /// </summary>
    /// <typeparam name="TEnum">The type of the enumeration.</typeparam>
    /// <typeparam name="TSeq">The integer sequence providing the values.
    /// </typeparam>
    template<class TEnum, class TSeq>
    struct cast_enum_sequence { };

    /// <summary>
    /// Specialisation unpacking an actual integer sequence.
    /// </summary>
    template<class TEnum, typename std::underlying_type<TEnum>::type... Values>
    struct cast_enum_sequence<TEnum, std::integer_sequence<
            typename std::underlying_type<TEnum>::type, Values...>>
        : enum_dispatch_list<TEnum, static_cast<TEnum>(Values)...> { };
}


    /// <summary>
    /// Auto-creates a <see cref="enum_dispatch_list" /> for continguous
    /// enumeration members valued [0, <typeparamref name="Max" />].
    /// </summary>
    /// <typeparam name="TEnum">The type of the enumeration.</typeparam>
    /// <typeparam name="Max">The value <i>after</i> the last valid one.
    /// </typeparam>
    template<class TEnum, TEnum Max>
    using contiguous_enum_dispatch_list = detail::cast_enum_sequence<TEnum,
        std::make_integer_sequence<typename std::underlying_type<TEnum>::type,
        static_cast<typename std::underlying_type<TEnum>::type>(Max)>>;
}
