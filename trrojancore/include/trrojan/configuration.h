/// <copyright file="configuration.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "trrojan/named_variant.h"


namespace trrojan {

    /// <summary>
    /// A configuration, which is defined as a set of manifestations of
    /// (named) factors.
    /// </summary>
    typedef std::vector<named_variant> configuration;

    /// <summary>
    /// Answer the configuration as name-value pairs in a string.
    /// </summary>
    std::string to_string(const configuration& c);

    /// <summary>
    /// Write a <see cref="trrojan::configuration" /> to a stream.
    /// </summary>
    /// <param name="lhs">The left-hand side operand (the stream to
    /// write to).</param>
    /// <param name="rhs">The right-hand side operand (the object to
    /// be written).</param>
    /// <returns><paramref name="lhs" />.</returns>
    /// <tparam name="C">The character type used in the stream.</tparam>
    /// <tparam name="T">The traits for <tparamref name="C" />.</tparam>
    template<class C, class T>
    inline std::basic_ostream<C, T>& operator <<(std::basic_ostream<C, T>& lhs,
            const configuration& rhs) {
        bool isFirst = true;

        for (auto& f : rhs) {
            if (isFirst) {
                isFirst = false;
            } else {
                lhs << ", ";
            }

            lhs << f;
        }

        return lhs;
    }
}
