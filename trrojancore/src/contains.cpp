// <copyright file="contains.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/contains.h"


/*
 * trrojan::contains
 */
bool trrojan::contains(const std::vector<std::string>& haystack,
        const std::string& needle) {
    auto it = std::find(haystack.begin(), haystack.end(), needle);
    return (it != haystack.end());
}
