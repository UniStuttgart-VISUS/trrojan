/// <copyright file="enum_parse_helper.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <sstream>
#include <stdexcept>

#include "trrojan/enum_dispatch_list.h"
#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Utility class for parsing enumeration values from strings.
    /// </summary>
    /// <tparam name="V">The type of the enumeration to be parsed.</tparam>
    /// <tparam name="T">The type of a traits class which allows for deriving
    /// the string representation of an enum member via a parameterless static
    /// method named <c>name</c>.</tparam>
    /// <tparam name="L">The type of a compile-time list expanding to the
    /// members of <tparamref name="V" /> to be recognised.</tparam>
    template<class V, template<V> class T, template<V...> class L>
    struct enum_parse_helper {

        typedef V enum_type;
        template<V... E> using list_type = L<E...>;
        template<V E> using traits_type = T<E>;

        template<enum_type E, enum_type... Es>
        static enum_type parse(list_type<E, Es...>, const std::string& str) {
            if (traits_type<E>::name() == str) {
                return E;
            } else {
                return enum_parse_helper::parse(list_type<Es...>(), str);
            }
        }

        static inline enum_type parse(list_type<>, const std::string& str) {
            std::stringstream msg;
            msg << "\"" << str << "\" is not a valid enumeration member."
                << std::ends;
            throw new std::invalid_argument(msg.str());
        }
    };

}
