/// <copyright file="configuration.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>
#include <vector>

#include "trrojan/export.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// A configuration, which is defined as a set of manifestations of
    /// (named) factors.
    /// </summary>
    typedef std::vector<std::pair<std::string, variant>> configuration;

}
