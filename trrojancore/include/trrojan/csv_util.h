// <copyright file="csv_output.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <iostream>

#include "trrojan/named_variant.h"


namespace trrojan {

    /// <summary>
    /// Print the name of the given variant as CSV (strings are quoted on
    /// request) to the given stream.
    /// </summary>
    /// <param name="stream"></param>
    /// <param name="variant"></param>
    /// <param name="quote"></param>
    /// <returns></returns>
    TRROJANCORE_API std::ostream& print_csv_header(std::ostream& stream,
        const named_variant& variant, const bool quote);

    /// <summary>
    /// Print the given variant as CSV (strings are quoted on request) to the
    /// given stream.
    /// </summary>
    /// <param name="stream"></param>
    /// <param name="variant"></param>
    /// <param name="quote"></param>
    /// <returns></returns>
    TRROJANCORE_API std::ostream& print_csv_value(std::ostream& stream,
        const variant& variant, const bool quote);

    /// <summary>
    /// Print the value of the given variant as CSV (strings are quoted on
    /// request) to the given stream.
    /// </summary>
    /// <param name="stream"></param>
    /// <param name="variant"></param>
    /// <param name="quote"></param>
    /// <returns></returns>
    TRROJANCORE_API std::ostream& print_csv_value(std::ostream& stream,
        const named_variant& variant, const bool quote);
}
