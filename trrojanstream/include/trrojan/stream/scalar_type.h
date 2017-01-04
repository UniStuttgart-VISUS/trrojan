/// <copyright file="scalar_type.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/stream/export.h"

#include <cinttypes>


namespace trrojan {
namespace stream {

    /// <summary>
    /// Possible types of scalars to be used in the streaming benchmark.
    /// </summary>
    enum class TRROJANSTREAM_API scalar_type {

        /// <summary>
        /// 32-bit floating point numbers.
        /// </summary>
        float32,

        /// <summary>
        /// 64-bit floating point numbers.
        /// </summary>
        float64,

        /// <summary>
        /// 32-bit integer numbers.
        /// </summary>
        int32,

        /// <summary>
        /// 64-bit integer numbers.
        /// </summary>
        int64
    };


    /// <summary>
    /// A traits class for inferring the scalar type from its enumeration
    /// member.
    /// </summary>
    template<scalar_type S> struct scalar_type_traits { };

    template<> struct scalar_type_traits<scalar_type::float32> {
        typedef float type;
    };

    template<> struct scalar_type_traits<scalar_type::float64> {
        typedef double type;
    };

    template<> struct scalar_type_traits<scalar_type::int32> {
        typedef std::int32_t type;
    };

    template<> struct scalar_type_traits<scalar_type::int64> {
        typedef std::int64_t type;
    };


    /// <summary>
    /// A traits class for inferring the
    /// <see cref="trrojan::stream::scalar_type" /> type from a C++ type.
    /// </summary>
    template<class T> struct scalar_reverse_traits { };

    template<> struct scalar_reverse_traits<float> {
        static const scalar_type type = scalar_type::float32;
    };

    template<> struct scalar_reverse_traits<double> {
        static const scalar_type type = scalar_type::float64;
    };

    template<> struct scalar_reverse_traits<std::int32_t> {
        static const scalar_type type = scalar_type::int32;
    };

    template<> struct scalar_reverse_traits<std::int64_t> {
        static const scalar_type type = scalar_type::int64;
    };


namespace detail {

    template<scalar_type...> struct scalar_type_list_t { };

    typedef scalar_type_list_t<scalar_type::float32, scalar_type::float64,
        scalar_type::int32, scalar_type::int64> scalar_type_list;

    template<template<scalar_type> class F, scalar_type T, scalar_type... U,
            class... P>
    void dispatch(scalar_type_list_t<T, U...>, const scalar_type type,
            P&&... params) {
        if (type == T) {
            F<T>::invoke(std::forward<P>(params)...);
        }
        dispatch<F>(scalar_type_list_t<U...>(), type,
            std::forward<P>(params)...);
    }

    template<template<scalar_type> class F, class... P>
    inline void dispatch(scalar_type_list_t<>, const scalar_type type,
        P&&... params) { }

} /* end namespace detail */


    /// <summary>
    /// Invokes the functor <tparamref name="F" /> for the scalar type
    /// <paramref name="type" />.
    /// </summary>
    template<template<scalar_type> class F, class... P>
    inline void scalar_type_dispatch(const scalar_type type, P&&... params) {
        detail::dispatch<F>(detail::scalar_type_list(), type,
            std::forward<P>(params)...);
    }

} /* end namespace stream */
} /* end namespace trrojan */
