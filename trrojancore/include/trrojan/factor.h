/// <copyright file="factor.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include "trrojan/export.h"
#include "trrojan/variant.h"


namespace trrojan {

    /* Forward declarations. */
    namespace detail { class factor_base; }


    /// <summary>
    /// This class represents a variable factor which impacts performance.
    /// </summary>
    /// <remarks>
    /// <para>We use the facade pattern in order to allow for the factor living
    /// on the stack and hiding different kinds of factors which implement the
    /// same functionality differently.</para>
    /// <remarks>
    class TRROJANCORE_API factor {

    public:

        /// <summary>
        /// Answer the number of different manifestations the factor has.
        /// </summary>
        /// <returns>The number of manifestations.</returns>
        size_t size(void) const;

        /// <summary>
        /// Answer a specific manifestation.
        /// </summary>
        /// <param name="i"></param>
        /// <returns>The <paramref name="i" />th manifestation.</returns>
        /// <exception cref="std::range_error"></exception>
        const variant& operator [](const size_t i) const;

    private:

        /// <summary>
        /// Pointer to the actual implementation.
        /// </summary>
        std::unique_ptr<detail::factor_base> impl;

    };
}
