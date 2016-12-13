/// <copyright file="factor.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "factor_base.h"


namespace trrojan {
namespace detail {

    /// <summary>
    /// Base class for different implementations of a
    /// <see cref="trrojan::factor" />.
    /// </summary>
    /// <tparam name="T">A numeric type that can be interpolated.</tparam>
    template<class T> class factor_range : public factor_base {

    public:

        virtual size_t size(void) const;

        virtual const variant& operator [](const size_t i) const;

    };
}
}

#include "factor_range.inl"
