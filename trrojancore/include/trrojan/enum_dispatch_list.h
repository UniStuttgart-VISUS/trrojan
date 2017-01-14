/// <copyright file="enum_dispatch_list.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


namespace trrojan {

    /// <summary>
    /// Template for dispatch lists which allow for defining lists of enum
    /// values.
    /// </summary>
    /// <remarks>
    /// The intended use of this class is the definition of a dispatch list type
    /// for an enumeration type <c>et</c> like this
    /// <c>template<et... V> using et_list_t = enum_dispatch_list<et, V...>;</c>
    /// </remarks>
    /// <tparam name="E">The type of the enumeration.</tparam>
    /// <tparam name="V">The enumeration values.</tparam>
    template<class E, E... V> struct enum_dispatch_list { };
}
