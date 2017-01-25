#include "text.h"
/// <copyright file="text.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::ends_with
 */
template<class T> bool trrojan::ends_with(const std::basic_string<T>& haystack,
        const std::basic_string<T>& needle) {
    if (needle.size() > haystack.size()) {
        return false;
    } else {
        return (haystack.substr(haystack.size() - needle.size()) == needle);
    }
}


/*
 * trrojan::join
 */
template<class... P> std::string trrojan::join(const std::string& sep,
        std::string str, P&&... strs) {
    int unpack[]{ 0, (str += sep + trrojan::join(sep, strs), 0)... };
    static_cast<void>(unpack);
    return str;
}


/*
 * trrojan::join
 */
template<class I> std::string trrojan::join(const std::string& sep,
        I begin, I end) {
    bool isFirst = true;
    std::string retval;

    for (auto it = begin; it != end; ++it) {
        if (isFirst) {
            isFirst = false;
        } else {
            retval += sep;
        }
        retval += *it;
    }

    return retval;
}


/*
 * trrojan::parse
 */
 template<class R, class T> R trrojan::parse(const T *str) {
    if (str == nullptr) {
        throw std::invalid_argument("The string to be parsed must not be "
            "nullptr.");
    }

    std::basic_stringstream<T> input(str);
    R retval;

    if (!(input >> retval)) {
        std::basic_stringstream<T> msg;
        msg << "\"" << str << "\" cannot be parsed as " << typeid(R).name()
            << std::ends;
        throw std::invalid_argument(msg.str());
    }

    return retval;
}


/*
 * trrojan::parse_bool
 */
template<class T> bool trrojan::parse_bool(const T *str) {
    if (str == nullptr) {
        throw std::invalid_argument("The string to be parsed must not be "
            "nullptr.");
    }

    auto s = trrojan::tolower(str);

    if ((s == "true") || (s == "yes")) {
        return true;

    } else if ((s == "false") || (s == "no")) {
        return false;

    } else {
        return (trrojan::parse<int>(str) != 0);
    }
}


/*
 * trrojan::starts_with
 */
template<class T>
bool trrojan::starts_with(const std::basic_string<T>& haystack,
        const std::basic_string<T>& needle) {
    if (needle.size() > haystack.size()) {
        return false;
    } else {
        return (haystack.substr(0, needle.size()) == needle);
    }
}
