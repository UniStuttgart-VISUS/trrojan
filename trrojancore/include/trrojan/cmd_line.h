/// <copyright file="configuration.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>


namespace trrojan {

    /// <summary>
    /// The representation of the command line parameters.
    /// </summary>
    typedef std::vector<std::string> cmd_line;

    /// <summary>
    /// Find the first occurrence of the specified switch in a range of
    /// command line arguments.
    /// <summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="end"></param>
    /// <param name="matchCase"></param>
    /// <tparam name="T"></tparam>
    /// <tparam name="I"></tparam>
    /// <returns></returns>
    template<class T, class I>
    inline I find_switch(const std::basic_string<T>& name, I begin, I end,
        const bool matchCase = false) {
        return std::find_if(begin, end, [&](const std::basic_string<T>& s) {
            if (matchCase) {
                return (name == s);
            } else {
                return std::equal(name.cbegin(), name.cend(), s.cbegin(),
                    [](T l, T r) { return (std::tolower(l) == std::tolower(r)); });
            }
        });
    }

    /// <summary>
    /// Answer whether the specified switch is in the range of the given command
    /// line arguments.
    /// <summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="end"></param>
    /// <param name="matchCase"></param>
    /// <tparam name="T"></tparam>
    /// <tparam name="I"></tparam>
    /// <returns></returns>
    template<class T, class I>
    inline bool contains_switch(const std::basic_string<T>& name, I begin,
            I end, const bool matchCase = false) {
        return (find_switch(name, begin, end, matchCase) != end);
    }

    /// <summary>
    /// Find the first occurrence of an argument with the specified switch name
    /// in a range of command line arguments.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="end"></param>
    /// <param name="matchCase"></param>
    /// <tparam name="T"></tparam>
    /// <tparam name="I"></tparam>
    /// <returns>An iterator of the first element after the switch ifself, or 
    /// <paramref name="end" /> if the respective argument has not been found.
    template<class T, class I>
    inline I find_argument(const std::basic_string<T>& name, I begin, I end,
            const bool matchCase = false) {
        auto retval = find_switch(name, begin, end, matchCase);
        return (retval != end) ? ++retval : retval;
    }

    /// <summary>
    /// Find the first occurrence of an argument with the specified switch name
    /// in a range of command line arguments.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="end"></param>
    /// <param name="matchCase"></param>
    /// <tparam name="T"></tparam>
    /// <tparam name="I"></tparam>
    /// <returns>An iterator of the first element after the switch ifself, or 
    /// <paramref name="end" /> if the respective argument has not been found.
    template<class T, class I>
    inline I find_argument(const T *name, I begin, I end,
            const bool matchCase = false) {
        return find_argument(std::basic_string<T>(name), begin, end, matchCase);
    }
}
