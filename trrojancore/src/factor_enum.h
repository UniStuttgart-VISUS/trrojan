/// <copyright file="factor_enum.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "factor_base.h"



namespace trrojan {
namespace detail {

    /// <summary>
    /// Implements a factor with discrete manifestations that are explicitly
    /// enumerated.
    /// </summary>
    class factor_enum : public factor_base {

    public:

        virtual size_t size(void) const;

        virtual const variant& operator [](const size_t i) const;

    };
}
}
