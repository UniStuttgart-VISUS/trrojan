/// <copyright file="result.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Valentin Bruder</author>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/named_variant.h"


namespace trrojan {

    /// <summary>
    /// A single result from a benchmark, which is uniquely identified by its
    /// name.
    /// </summary>
    typedef named_variant result;
}
