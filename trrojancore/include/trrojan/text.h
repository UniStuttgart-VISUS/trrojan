/// <copyright file="text.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <string>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Answer whether <paramref name="haystack" /> ends with
    /// <see cref="needle" />.
    /// </summary>
    template<class T>
    inline bool TRROJANCORE_API ends_with(const std::basic_string<T>& haystack,
            const std::basic_string<T>& needle) {
        if (needle.size() > haystack.size()) {
            return false;
        } else {
            return (haystack.substr(haystack.size() - needle.size()) == needle);
        }
    }

    /// <summary>
    /// Answer whether <paramref name="haystack" /> starts with
    /// <see cref="needle" />.
    /// </summary>
    template<class T>
    inline bool TRROJANCORE_API starts_with(const std::basic_string<T>& haystack,
            const std::basic_string<T>& needle) {
        if (needle.size() > haystack.size()) {
            return false;
        } else {
            return (haystack.substr(0, needle.size()) == needle);
        }
    }

    /// <summary>
    /// Remove all leading white-space characters from <paramref name="str" />.
    /// </summary>
    /// <param name="str"></param>
    /// <tparam name="T"></tparam>
    /// <returns></returns>
    template<class T> inline std::basic_string<T> TRROJANCORE_API trim_left(
            const std::basic_string<T>& str) {
        // http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
        auto end = str.cend();
        auto begin = std::find_if(str.cbegin(), end,
            std::not1(std::ptr_fun<int, int>(std::isspace)));
        return std::basic_string<T>(begin, str.cend());
    }

    /// <summary>
    /// Remove all trailing white-space characters from <paramref name="str" />.
    /// </summary>
    /// <param name="str"></param>
    /// <tparam name="T"></tparam>
    /// <returns></returns>
    template<class T> inline std::basic_string<T> TRROJANCORE_API trim_right(
            const std::basic_string<T>& str) {
        // http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
        auto end = std::find_if(str.crbegin(), str.crend(),
            std::not1(std::ptr_fun<int, int>(std::isspace)));
        return std::basic_string<T>(str.cbegin(), end.base());
    }

    /// <summary>
    /// Remove all leading and trailing white-space characters from
    /// <paramref name="str" />.
    /// </summary>
    /// <param name="str"></param>
    /// <tparam name="T"></tparam>
    /// <returns></returns>
    template<class T> inline std::basic_string<T> TRROJANCORE_API trim(
            const std::basic_string<T>& str) {
        return trim_left(trim_right(str));
    }
}
