// <copyright file="utilities.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d12::unsmart
 */
template<class T>
std::vector<T *> trrojan::d3d12::unsmart(std::vector<winrt::com_ptr<T>>& input) {
    std::vector<T *> retval(input.size());
    std::transform(input.begin(), input.end(), retval.begin(),
        [](winrt::com_ptr<T>& i) { return i.get(); });
    return retval;
}
