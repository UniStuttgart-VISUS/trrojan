/// <copyright file="variant.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::detail::copy_to::invoke
 */
template<trrojan::variant_type T>
void trrojan::detail::copy_to<T>::invoke(type& v, trrojan::variant& target) {
    target.set(v);
}


/*
 * trrojan::detail::is_same::invoke
 */
template<trrojan::variant_type T>
void trrojan::detail::is_same<T>::invoke(type& v, const trrojan::variant& rhs,
        bool& retval) {
    retval = (v == rhs.get<T>());
}


/*
 * trrojan::detail::move_to::invoke
 */
template<trrojan::variant_type T>
void trrojan::detail::move_to<T>::invoke(type& v, trrojan::variant& target) {
    target.set(std::move(v));
}


/*
 * trrojan::variant::conditional_invoke0
 */
template<template<trrojan::variant_type> class F, trrojan::variant_type T,
    trrojan::variant_type... U, class... P>
void trrojan::variant::conditional_invoke0(detail::variant_type_list_t<T, U...>,
        P&&... params) {
    if (this->cur_type == T) {
        F<T>::invoke(*variant_type_traits<T>::get(this->data),
            std::forward<P>(params)...);
    }
    this->conditional_invoke0<F>(detail::variant_type_list_t<U...>(),
        std::forward<P>(params)...);
}
