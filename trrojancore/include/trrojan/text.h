/// <copyright file="text.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#include <algorithm>
#include <cctype>
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
#include <codecvt>
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
#include <functional>
#include <iterator>
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
    /// Convert an UTF-8 string to a wide string.
    /// </summary>
    inline std::wstring from_utf8(const std::string& str) {
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        return cvt.from_bytes(str);
#else /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
        std::wstring retval;
        for (auto c : str) {
            retval += static_cast<wchar_t>(c);
        }
        return retval;
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
    }

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
        std::transform(str.cbegin(), str.cend(), std::back_inserter(retval),
            ::tolower);
        return retval;
    }

    /// <summary>
    /// Convert a wide string to a UTF-8 string.
    /// </summary>
    inline std::string to_utf8(const std::wstring& str) {
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        return cvt.to_bytes(str);
#else /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
        std::string retval;
        for (auto c : str) {
            retval += static_cast<char>(c);
        }
        return retval;
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
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
