// <copyright file="trroll_parser.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "trrojan/configuration_set.h"
#include "trrojan/text.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// Parses <see cref="trrojan::configuration_set" />s from
    /// "TRRojan Lacklustre Language" configuration files.
    /// </summary>
    class TRROJANCORE_API trroll_parser {

    public:

        /// <summary>
        /// The benchmark along with the configurations that should be run
        /// on it.
        /// </summary>
        struct benchmark_configs {
            std::string plugin;
            std::string benchmark;
            configuration_set configs;
        };

        /// <summary>
        /// Defines the type used to pass a troll file to the executive.
        /// </summary>
#if defined(TRROJAN_FOR_UWP)
        typedef winrt::Windows::Storage::StorageFile troll_input_type;
#else /* defined(TRROJAN_FOR_UWP) */
        typedef std::string troll_input_type;
#endif /* defined(TRROJAN_FOR_UWP) */

        static std::vector<benchmark_configs> parse(
            const troll_input_type& path);

        trroll_parser(void) = delete;

    private:

        /// <summary>
        /// Tries parsing <paramref name="str" /> as a
        /// <see cref="variant_type" /> using the name provided in the traits.
        /// </summary>
        static variant_type parse_type(const std::string& str);

        template<variant_type T, variant_type... Ts>
        static variant parse_value(detail::variant_type_list_t<T, Ts...>,
            const std::string& str, const variant_type type);

        static variant parse_value(detail::variant_type_list_t<>,
            const std::string& str, const variant_type type);

        /// <summary>
        /// Tries parsing <paramref name="str" /> as <typeparamref name="T" />.
        /// </summary>
        template<variant_type T>
        static inline typename std::enable_if<variant_type_traits<T>::parsable,
                variant>::type parse_value(const std::string& str) {
            return trrojan::parse<typename variant_type_traits<T>::type>(str);
        }

        /// <summary>
        /// Fails parsing <paramref name="str" /> as <typeparamref name="T" />
        /// for all types not marked parsable.
        /// </summary>
        template<variant_type T>
        static inline typename std::enable_if<!variant_type_traits<T>::parsable,
                variant>::type parse_value(const std::string& str) {
            // Instantly fail, because we hit a non-parsable type.
            return trroll_parser::parse_value(detail::variant_type_list_t<>(),
                str, T);
        }

        template<class I> static I skip_nonspaces(I it, I end);

        template<class I> static I skip_spaces(I it, I end);

        template<variant_type T, variant_type... Ts>
        static std::string to_string(detail::variant_type_list_t<T, Ts...>,
            const variant_type type);

        static std::string to_string(detail::variant_type_list_t<>,
                const variant_type type) {
            return std::string();
        }

        template<class Is, class P>
        static std::vector<Is> tokenise(Is begin, Is end, P sepPred);

        template<class Is, class It>
        static std::vector<Is> tokenise(Is begin, Is end,
            It sepBegin, It sepEnd);

    };
}

#include "trrojan/trroll_parser.inl"
