/// <copyright file="text.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <locale>
#include <sstream>
#include <string>
#include <stdexcept>


namespace trrojan {

    /// <summary>
    /// Answer whether <paramref name="haystack" /> ends with
    /// <see cref="needle" />.
    /// </summary>
    template<class T>
    bool ends_with(const std::basic_string<T>& haystack,
        const std::basic_string<T>& needle);

    /// <summary>
    /// Joins a set of strings with the given separator between them.
    /// </summary>
    template<class... P> std::string join(
        const std::string& sep, std::string str, P&&... strs);

    /// <summary>
    /// Joins a range of strings with the given separator between them.
    /// </summary>
    template<class I> std::string join(
        const std::string& sep, I begin, I end);

    /// <summary>
    /// Parses the given string <paramref name="str" /> as type
    /// <tparamref name="R" />.
    /// </summary>
    /// <param name="str">The string to be parsed.</param>
    /// <returns>The parsed value.</returns>
    /// <exception cref="std::invalid_argument">If the string could
    /// not be parsed or if <paramref name="str" /> is <c>nullptr</c>.</param>
    template<class R, class T> R parse(const T *str);

    /// <summary>
    /// Parses the given string <paramref name="str" /> as type
    /// <tparamref name="R" />.
    /// </summary>
    /// <param name="str">The string to be parsed.</param>
    /// <returns>The parsed value.</returns>
    /// <exception cref="std::invalid_argument">If the string could
    /// not be parsed.</param>
    template<class R, class C, class T, class A>
    inline R parse(const std::basic_string<C, T, A>& str) {
        return parse<R, C>(str.c_str());
    }

    /// <summary>
    /// Parses the given string <paramref name="str" /> as type Boolean
    /// value ("true", "false" or numeric value).
    /// </summary>
    template<class T> bool parse_bool(const T *str);

    /// <summary>
    /// Parses the given string <paramref name="str" /> as type Boolean
    /// value ("true", "false" or numeric value).
    /// </summary>
    template<class C, class T, class A>
    inline bool parse_bool(const std::basic_string<C, T, A>& str) {
        return parse_bool<C>(str.c_str());
    }

    /// <summary>
    /// Answer whether <paramref name="haystack" /> starts with
    /// <see cref="needle" />.
    /// </summary>
    template<class T>
    bool starts_with(const std::basic_string<T>& haystack,
        const std::basic_string<T>& needle);

    /// <summary>
    /// Convert <paramref name="str" /> to lower-case.
    /// </summary>
    template<class T> std::basic_string<T> tolower(const T *str) {
        std::basic_string<T> retval(str);
        std::transform(retval.begin(), retval.end(), retval.begin(), ::tolower);
        return retval;
    }

    /// <summary>
    /// Convert <paramref name="str" /> to lower-case.
    /// </summary>
    template<class T>
    std::basic_string<T> tolower(const std::basic_string<T>& str) {
        std::basic_string<T> retval;
        std::transform(str.cbegin(), str.cend(), retval.begin(), ::tolower);
        return retval;
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
    template<class T> inline std::basic_string<T> trim_right(
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
    template<class T> inline std::basic_string<T> trim(
            const std::basic_string<T>& str) {
        return trim_left(trim_right(str));
    }
}

#include "trrojan/text.inl"
