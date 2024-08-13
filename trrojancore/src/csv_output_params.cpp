/// <copyright file="csv_output_params.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
