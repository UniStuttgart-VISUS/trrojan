/// <copyright file="r_output_params.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
