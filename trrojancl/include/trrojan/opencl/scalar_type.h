/// <copyright file="scalar_type.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>
/// <author>Valentin Bruder</author>

#pragma once

#include <cinttypes>
#include <string>

#include "trrojan/enum_dispatch_list.h"

#include "trrojan/opencl/export.h"


namespace trrojan
{
namespace opencl
{

    /// <summary>
    /// Possible types of scalars to be used in the streaming benchmark.
    /// </summary>
    enum class TRROJANCL_API scalar_type
    {
        /// <summary>
        /// 1-byte integer type
        /// </summary>
        uchar,

        /// <summary>
        /// 2-byte integer type
        /// </summary>
        ushort,

        /// <summary>
        /// 4-byte floating point type
        /// </summary>
        float32,

        /// <summary>
        /// 8-byte floating point type (double precision)
        /// </summary>
        float64
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

    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(uchar, unsigned char);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(ushort, unsigned short);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(float32, float);
    __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS(float64, double);

#undef __TRROJANCORE_DECL_SCALAR_TYPE_TRAITS


    template<scalar_type... V>
    using scalar_type_list_t = enum_dispatch_list<scalar_type, V...>;

    typedef scalar_type_list_t<scalar_type::uchar, scalar_type::ushort,
        scalar_type::float32, scalar_type::float64> scalar_type_list;

} /* end namespace stream */
} /* end namespace trrojan */
