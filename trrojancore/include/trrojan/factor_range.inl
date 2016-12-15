/// <copyright file="factor_range.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::detail::factor_range<T>::~factor_range
 */
template<class T>
trrojan::detail::factor_range<T>::~factor_range(void) { }


/*
 * trrojan::detail::factor_range<T>::size
 */
template<class T>
size_t trrojan::detail::factor_range<T>::size(void) const {
    return this->cnt_steps;
}


/*
 * trrojan::detail::factor_range<T>::operator []
 */
template<class T>
const trrojan::variant& trrojan::detail::factor_range<T>::operator [](
        const size_t i) const {
    if (i >= this->size()) {
        throw std::range_error("The factor does not contain the requested "
            "manifestation.");
    }

    return (this->begin + i * this->step_size);
}
