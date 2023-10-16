// <copyright file="contains.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::contains
 */
template<std::size_t N>
bool trrojan::contains(const std::array<std::string, N>& haystack,
        const std::string& needle) {
    auto it = std::find(haystack.begin(), haystack.end(), needle);
    return (it != haystack.end());
}


/*
 * trrojan::contains_any
 */
template<class... TNeedle>
bool trrojan::contains_any<TNeedle...>(const std::vector<std::string>& haystack,
        TNeedle&&... needles) {
    std::array<std::string, sizeof...(TNeedle)> n = { needles... };
    auto it = std::find_if(haystack.begin(), haystack.end(),
        [&n](const std::string h) { return contains(n, h); });
    return (it != haystack.end());
}
