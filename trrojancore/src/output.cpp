// <copyright file="output.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/output.h"

#include "trrojan/console_output.h"
#include "trrojan/console_output_params.h"
#include "trrojan/csv_output.h"
#include "trrojan/csv_output_params.h"
#include "trrojan/excel_output.h"
#include "trrojan/excel_output_params.h"
#include "trrojan/log.h"
#include "trrojan/r_output.h"
#include "trrojan/r_output_params.h"
#include "trrojan/text.h"


/*
 * trrojan::output_base::~output_base
 */
trrojan::output_base::~output_base(void) { }


/*
 * trrojan::output_base::operator <<
 */
trrojan::output_base& trrojan::output_base::operator <<(const result result) {
    return (result) ? (*this << *result) : *this;
}


/*
 * trrojan::output_base::operator <<
 */
trrojan::output_base& trrojan::output_base::operator <<(
        const result_set& results) {
    for (auto& r : results) {
        *this << r;
    }
    return *this;
}


/*
 * trrojan::make_output
 */
trrojan::output TRROJANCORE_API trrojan::make_output(const std::string& path) {
    auto idx = path.rfind('.');
    auto ext = tolower(path.substr(idx));

    if (iequals(ext, std::string(".csv"))
            || iequals(ext, std::string(".tsv"))
            || iequals(ext, std::string(".txt"))) {
        return std::make_shared<csv_output>();
#if defined(_WIN32) && !defined(_UWP)
    } else if (iequals(ext, std::string(".xlsx"))) {
        return std::make_shared<excel_output>();
#endif /* defined(_WIN32) && !defined(_UWP) */
    } else if (iequals(ext, std::string(".r"))) {
        return std::make_shared<r_output>();
    } else {
        log::instance().write_line(log_level::warning, "The file name "
            "extension \"{0}\" of path \"{1}\" cannot be use to determine "
            "the output type.", ext, path);
        return nullptr;
    }
}


/*
 * trrojan::output trrojan::open_output
 */
trrojan::output trrojan::open_output(const trrojan::cmd_line& cmdLine) {
    trrojan::output retval;
    trrojan::output_params params;

    auto output = trrojan::find_argument("--output", cmdLine.begin(),
        cmdLine.end());
    if (output != cmdLine.end()) {
        retval = make_output(*output);
    } else {
        log::instance().write_line(trrojan::log_level::warning, "You have not "
            "specified an output file. Please do so using the --output "
            "option.");
    }

    if (retval == nullptr) {
        // Note: this is not in the else branch above on purpose, because
        // make_output might fail, too, depending on the output file name.
        retval = std::make_shared<console_output>();
    }

    // Create matching parameters.
    if (std::dynamic_pointer_cast<csv_output>(retval) != nullptr) {
        params = basic_output_params::create<csv_output_params>(*output,
            cmdLine.begin(), cmdLine.end());

#if defined(_WIN32) && !defined(_UWP)
    } else if (std::dynamic_pointer_cast<excel_output>(retval) != nullptr) {
        params = basic_output_params::create<excel_output_params>(*output,
            cmdLine.begin(), cmdLine.end());
#endif /* defined(_WIN32) && !defined(_UWP) */

    } else if (std::dynamic_pointer_cast<r_output>(retval) != nullptr) {
        params = basic_output_params::create<r_output_params>(*output,
            cmdLine.begin(), cmdLine.end());

    } else if (std::dynamic_pointer_cast<console_output>(retval) != nullptr) {
        params = console_output_params::create();
    }

    retval->open(params);

    return retval;
}

