/// <copyright file="trroll_parser.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright � 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

#include "trrojan/configuration_set.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// Parses <see cref="trrojan::configuration_set" />s from
    /// "TRRojan Lacklustre Language" configuration files.
    /// </summary>
    class TRROJANCORE_API trroll_parser {

    public:

        struct benchmark_configs {
            std::string plugin;
            std::string benchmark;
            configuration_set configs;
        };

        static std::vector<benchmark_configs> parse(const std::string& path);

        trroll_parser(void) = delete;

    private:

        static variant_type parse_type(const std::string& str);

        template<variant_type T, variant_type... Ts>
        static variant parse_value(detail::variant_type_list_t<T, Ts...>,
            const std::string& str, const variant_type type);

        static variant parse_value(detail::variant_type_list_t<>,
            const std::string& str, const variant_type type);

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
