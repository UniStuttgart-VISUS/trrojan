// <copyright file="contains.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <array>
#include <string>
#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Answer whether the array <paramref name="haystack" /> contains
    /// <paramref name="needle" />.
    /// </summary>
    /// <param name="haystack"></param>
    /// <param name="needle"></param>
    /// <returns></returns>
    TRROJANCORE_API extern bool contains(
        const std::vector<std::string>& haystack,
        const std::string& needle);

    /// <summary>
    /// Answer whether the array <paramref name="haystack" /> contains
    /// <paramref name="needle" />.
    /// </summary>
    /// <param name="haystack"></param>
    /// <param name="needle"></param>
    /// <returns></returns>
    template<std::size_t N>
    bool contains(const std::array<std::string, N>& haystack,
        const std::string& needle);

    /// <summary>
    /// Answer whether the array <paramref name="haystack" /> contains any
    /// of <paramref name="needles" />.
    /// </summary>
    /// <typeparam name="TNeedle"></typeparam>
    /// <param name="haystack"></param>
    /// <param name="needles"></param>
    /// <returns></returns>
    template<class... TNeedle>
    static bool contains_any(const std::vector<std::string>& haystack,
        TNeedle&&... needles);

}

#include "trrojan/contains.inl"
