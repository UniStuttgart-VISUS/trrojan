// <copyright file="text.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


namespace trrojan {
namespace detail {

    /// <summary>
    /// Supports parsing space-separated arrays from strings.
    /// </summary>
    template<class C, class T, size_t S>
    std::basic_stringstream<C>& operator >>(std::basic_stringstream<C>& lhs,
            std::array<T, S>& rhs) {
        for (size_t i = 0; i < S; ++i) {
            while (std::isspace(lhs.get()));
            lhs.unget();
            if (!(lhs >> rhs[i])) {
                std::stringstream msg;
                msg << "An array element could not be parsed as "
                    << typeid(T).name() << std::ends;
                throw std::invalid_argument(msg.str());
            }
        }
        return lhs;
    }

} /* end namespace detail */
} /* end namespace trrojan */


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
template<class I>
typename std::enable_if<!std::is_same<std::string,
    typename std::decay<I>::type>::value, std::string>::type
trrojan::join(const std::string& sep, I begin, I end) {
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
    using namespace trrojan::detail;
    if (str == nullptr) {
        throw std::invalid_argument("The string to be parsed must not be "
            "nullptr.");
    }

    std::basic_stringstream<T> input(str);
    R retval;

    if (!(input >> retval)) {
        std::stringstream msg;
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
