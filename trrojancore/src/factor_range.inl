/// <copyright file="factor_range.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::detail::factor_range<T>::size
 */
template<class T>
size_t trrojan::detail::factor_range<T>::size(void) const {
    throw 1;
}


/*
 * trrojan::detail::factor_range<T>::operator []
 */
template<class T>
const trrojan::variant& trrojan::detail::factor_range<T>::operator [](
        const size_t i) const {
    throw 1;
}
