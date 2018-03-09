/// <copyright file="r_output_params.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/r_output_params.h"


/*
 * trrojan::r_output_params::default_line_break
 */
const std::string trrojan::r_output_params::default_line_break(
#ifdef _WIN32
    "\r\n"
#else /* _WIN32 */
    "\n"
#endif /* _WIN32 */
);


/*
 * trrojan::r_output_params::default_variable_name
 */
const std::string trrojan::r_output_params::default_variable_name("trrojan");
