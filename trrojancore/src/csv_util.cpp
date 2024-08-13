// <copyright file="csv_util.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/csv_util.h"


/*
 * trrojan::print_csv_header
 */
std::ostream& trrojan::print_csv_header(std::ostream& stream,
        const named_variant& variant, const bool quote) {
    if (quote) {
        stream << "\"" << variant.name() << "\"";
    } else {
        stream << variant.name();
    }

    return stream;
}


/*
 * trrojan::print_csv_value
 */
std::ostream& trrojan::print_csv_value(std::ostream& stream,
        const variant& variant, const bool quote) {
    bool isString;

    switch (variant.type()) {
        case variant_type::string:
        case variant_type::wstring:
        case variant_type::device:
        case variant_type::environment:
        case variant_type::power_collector:
            isString = true;
            break;

        default:
            isString = false;
    }

    if (quote && isString) {
        stream << "\"" << variant << "\"";
    } else {
        stream << variant;
    }

    return stream;
}


/*
 * trrojan::print_csv_value
 */
std::ostream& trrojan::print_csv_value(std::ostream& stream,
        const named_variant& variant, const bool quote) {
    return print_csv_value(stream, variant.value(), quote);
}
