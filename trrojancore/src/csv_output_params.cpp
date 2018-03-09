/// <copyright file="csv_output_params.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/csv_output_params.h"


/*
 * trrojan::csv_output_params::default_line_break
 */
const std::string trrojan::csv_output_params::default_line_break(
#ifdef _WIN32
    "\r\n"
#else /* _WIN32 */
    "\n"
#endif /* _WIN32 */
);


/*
 * trrojan::csv_output_params::default_separator
 */
const std::string trrojan::csv_output_params::default_separator("\t");


/*
 * trrojan::csv_output_params::~csv_output_params
 */
trrojan::csv_output_params::~csv_output_params(void) { }


/*
 * trrojan::csv_output_params::unescape
 */
std::string trrojan::csv_output_params::unescape(
        std::string::const_iterator begin, std::string::const_iterator end) {
    std::string retval;
    retval.reserve(std::distance(begin, end));

    while (begin != end) {
        auto c = *begin++;

        if ((c == '\\') && (begin != end)) {
            auto e = *begin++;

            switch (e) {
                case '\\': c = '\\'; break;
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                default: c = e; break;
            }
        }

        retval += c;
    }

    return retval;
}
