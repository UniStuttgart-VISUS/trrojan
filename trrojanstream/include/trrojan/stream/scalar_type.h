/// <copyright file="scalar_type.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <string>

#include "trrojan/enum_dispatch_list.h"

#include "trrojan/stream/export.h"


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

    /// <summary>
    /// A traits class for inferring the
    /// <see cref="trrojan::stream::scalar_type" /> type from a C++ type.
    /// </summary>
    template<class T> struct scalar_reverse_traits { };

#define __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(t0, t1)                          \
    template<> struct scalar_type_traits<scalar_type::t0> {                    \
        typedef t1 type;                                                       \
        static inline const std::string& name(void) {                          \
            static const std::string retval(#t0);                              \
            return retval;                                                     \
        }                                                                      \
    };                                                                         \
    template<> struct scalar_reverse_traits<t1> {                              \
        static const scalar_type type = scalar_type::t0;                       \
        static inline const std::string& name(void) {                          \
            static const std::string retval(#t0);                              \
            return retval;                                                     \
        }                                                                      \
    }

    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(float32, float);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(float64, double);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(int32, std::int32_t);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(int64, std::int64_t);

#undef __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS


    template<scalar_type... V>
    using scalar_type_list_t = enum_dispatch_list<scalar_type, V...>;

    typedef scalar_type_list_t<scalar_type::float32, scalar_type::float64,
        scalar_type::int32, scalar_type::int64> scalar_type_list;

} /* end namespace stream */
} /* end namespace trrojan */
