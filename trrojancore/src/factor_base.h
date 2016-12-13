/// <copyright file="factor.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan\variant.h"


namespace trrojan {
namespace detail {

    /// <summary>
    /// Interface of the different implementations of
    /// <see cref="trrojan::factor" />.
    /// </summary>
    class factor_base {

    public:

        virtual size_t size(void) const = 0;

        virtual const variant& operator [](const size_t i) const = 0;

    };
}
}
