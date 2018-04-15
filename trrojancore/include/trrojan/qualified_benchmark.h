/// <copyright file="qualified_benchmark.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/export.h"
#include "trrojan/benchmark.h"
#include "trrojan/plugin.h"


namespace trrojan {

    /// <summary>
    /// A container for returning a <see cref="trrojan::benchmark" />
    /// along with its containing <see cref="trrojan::plugin" />.
    /// </summary>
    struct TRROJANCORE_API qualified_benchmark {
        trrojan::plugin plugin;
        trrojan::benchmark benchmark;
    };

}
