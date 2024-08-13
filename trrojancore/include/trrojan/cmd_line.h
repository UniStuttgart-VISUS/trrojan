// <copyright file="cmd_line.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

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
                return equals(name, s);
            } else {
                return iequals(name, s);
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
    inline bool contains_switch(const T *name, I begin, I end,
            const bool matchCase = false) {
        return contains_switch(std::basic_string<T>(name), begin, end,
            matchCase);
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
