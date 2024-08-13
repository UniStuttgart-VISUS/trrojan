// <copyright file="utilities.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
// Copyright � 2016 - 2022 Visualisierungsinstitut der Universit�t Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph M�ller</author>


/*
 * trrojan::d3d12::unsmart
 */
template<class T>
std::vector<T *> trrojan::d3d12::unsmart(std::vector<ATL::CComPtr<T>>& input) {
    std::vector<T *> retval(input.size());
    std::transform(input.begin(), input.end(), retval.begin(),
        [](ATL::CComPtr<T>& i) { return i.p; });
    return retval;
}
