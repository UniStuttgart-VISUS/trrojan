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
