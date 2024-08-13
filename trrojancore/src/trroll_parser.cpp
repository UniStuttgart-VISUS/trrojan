// <copyright file="trroll_parser.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/trroll_parser.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

#include "trrojan/io.h"
#include "trrojan/enum_parse_helper.h"


/*
 * trrojan::trroll_parser::parse
 */
std::vector<trrojan::trroll_parser::benchmark_configs>
trrojan::trroll_parser::parse(const troll_input_type& path) {
    static const char NF[] = { ';' };
    static const char NL[] = { '\r', '\n' };
    static const char CL = '#';
    static const std::string NS("::");
    std::vector<benchmark_configs> retval;
    const auto content = trrojan::read_text_file(path);
    const auto trroll = trrojan::trim(content);

    auto cur = trroll.cbegin();
    const auto end = trroll.cend();
    while (cur != end) {
        auto beginBlock = std::find(cur, end, '{');
        auto endBlock = std::find(beginBlock, end, '}');
        if ((beginBlock == end) || (endBlock == end)) {
            throw std::runtime_error("The definition of the configurations of "
                "a benchmark must be enclosed in braces.");
        }

        /* Determine the bounds of the benchmark name. */
        auto benchName = trrojan::trim(std::string(cur, beginBlock));
        auto piEnd = benchName.find(NS, 0);
        if (piEnd == std::string::npos) {
            std::stringstream msg;
            msg << "\"" << benchName << "\" is not a valid benchmark name. A "
                "valid test name must have the form plugin::benchmark"
                << std::ends;
            throw std::runtime_error(msg.str());
        }

        /* Parse the factors into a configuration_set. */
        configuration_set cs;
        ++beginBlock;   // Skip the '{'
        auto blockTokens = trroll_parser::tokenise(beginBlock, endBlock,
            NL, NL + sizeof(NL) / sizeof(*NL));

        assert(blockTokens.size() > 0);
        for (size_t i = 0; i < blockTokens.size() - 1; ++i) {
            const auto eol = blockTokens[i + 1];        // End of line.
            const auto bol = trroll_parser::skip_spaces(blockTokens[i], eol);
            variant_type type = variant_type::empty;    // Type of factor.
            std::string name;                           // Name of factor.
            std::vector<variant> manifestations;        // Manifestations.

            if (bol == eol) {
                continue;  // Skip empty line.
            }
            if (*bol == CL) {
                continue;  // Skip comment line.
            }

            auto c = bol;

            /* Parse the type. */
            {
                c = trroll_parser::skip_spaces(c, eol);
                auto d = trroll_parser::skip_nonspaces(c, eol);
                auto token = std::string(c, d);
                c = d;
                type = trroll_parser::parse_type(token);
            }

            /* Find the assignment sign. */
            auto assignment = std::find(c, eol, '=');

            /* Parse the name. */
            {
                c = trroll_parser::skip_spaces(c, assignment);
                if (c == assignment) {
                    std::stringstream msg;
                    msg << "Expected name of factor in line \""
                        << std::string(bol, eol) << "\" before \""
                        << std::string(c, eol) << "\"." << std::ends;
                    throw std::runtime_error(msg.str());
                }
                auto d = trroll_parser::skip_nonspaces(c, eol);
                auto token = std::string(c, d);
                c = d;
                name = token;
            }

            /* Check assignment and continue after it. */
            if (assignment == eol) {
                std::stringstream msg;
                msg << "Expected \"=\" in line \""
                    << std::string(bol, eol) << "\" after \""
                    << std::string(bol, c) << "\"." << std::ends;
                throw std::runtime_error(msg.str());
            } else {
                c = assignment;
                ++c;
            }

            /* Parse the manifestations. */
            {
                c = trroll_parser::skip_spaces(c, eol);
                auto m = trroll_parser::tokenise(c, eol,
                    NF, NF + sizeof(NF) / sizeof(*NF));
                assert(m.size() > 0);
                for (size_t j = 0; j < m.size() - 1; ++j) {
                    std::string token(m[j], m[j + 1]);
                    std::replace(token.begin(), token.end(), NF[0], ' ');
                    token = trrojan::trim(token);
                    if (!token.empty()) {
                        append(manifestations, trroll_parser::parse_values(
                            detail::variant_type_list(), token, type));
                    }
                }
            }

            cs.add_factor(factor::from_manifestations(name, manifestations));

        } /* end for (size_t i = 0; i < blockTokens.size() - 1; ++i) */

        /* Add the benchmark to the result. */
        {
            benchmark_configs bc = {
                benchName.substr(0, piEnd),
                benchName.substr(piEnd + NS.length()),
                std::move(cs)
            };
            retval.push_back(std::move(bc));
        }

        cur = ++endBlock;      // Skip the '}'
    }

    return retval;
}


/*
 * trrojan::trroll_parser::parse_type
 */
trrojan::variant_type trrojan::trroll_parser::parse_type(
        const std::string& str) {
    typedef enum_parse_helper<variant_type, variant_type_traits,
        detail::variant_type_list_t> parser;
    return parser::parse(detail::variant_type_list(), str);
}


/*
 * trrojan::trroll_parser::parse_values
 */
std::vector<trrojan::variant> trrojan::trroll_parser::parse_values(
        detail::variant_type_list_t<>,
        const std::string& str,
        const variant_type type) {
    std::stringstream msg;
    msg << "Unable to parse variant value \"" << str << "\" as "
        << trroll_parser::to_string(detail::variant_type_list(), type)
        << "." << std::ends;
    throw std::runtime_error(msg.str());
}


/*
 * trrojan::trroll_parser::tokenise
 */
std::vector<std::string> trrojan::trroll_parser::tokenise(
        const std::string& str, const std::string& delimiter) {
    std::vector<std::string> retval;

    std::size_t cur = 0;
    for (; cur + delimiter.size() < str.size();) {
        auto next = str.find(delimiter, cur);

        if (next == std::string::npos) {
            next = str.size();
        }

        retval.push_back(str.substr(cur, next - cur));
        cur = next + delimiter.size();
    }

    if (cur + 1 < str.size()) {
        retval.push_back(str.substr(cur, str.size() - cur));
    }

    return retval;
}
