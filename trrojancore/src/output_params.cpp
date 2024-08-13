/// <copyright file="output_params.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/output_params.h"


/*
 * trrojan::basic_output_params::unescape
 */
std::string trrojan::basic_output_params::unescape(
        std::string::const_iterator begin, std::string::const_iterator end) {
    std::string retval;
    retval.reserve(std::distance(begin, end));

    while (begin != end) {
        auto c = *begin++;

        if ((c == '\\') && (begin != end)) {
            auto e = *begin++;

            switch (e) {
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'v': c = '\v'; break;
                case '\\': c = '\\'; break;
                default: c = e; break;
            }
        }

        retval += c;
    }

    return retval;
}
