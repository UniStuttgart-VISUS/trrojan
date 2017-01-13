/// <copyright file="configuration.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/configuration.h"

#include <sstream>


/*
 * trrojan::to_string
 */
std::string trrojan::to_string(const configuration& c) {
    std::stringstream retval;
    retval << c << std::ends;
    return retval.str();
}
