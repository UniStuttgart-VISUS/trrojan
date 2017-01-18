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
