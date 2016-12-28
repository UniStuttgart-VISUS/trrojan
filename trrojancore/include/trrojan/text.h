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


namespace trrojan {

    /// <summary>
    /// Remove all leading white-space characters from <paramref name="str" />.
    /// </summary>
    /// <param name="str"></param>
    /// <tparam name="T"></tparam>
    /// <returns></returns>
    template<class T>
    inline std::basic_string<T> trim_left(const std::basic_string<T>& str) {
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
    template<class T>
    inline std::basic_string<T> trim_right(const std::basic_string<T>& str) {
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
    template<class T>
    inline std::basic_string<T> trim(const std::basic_string<T>& str) {
        return trim_left(trim_right(str));
    }
}
