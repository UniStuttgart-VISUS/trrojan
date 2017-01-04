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
        static const size_t size = sizeof(float);
        typedef float type;
    };

    template<> struct scalar_type_traits<scalar_type::float64> {
        static const size_t size = sizeof(double);
        typedef double type;
    };

    template<> struct scalar_type_traits<scalar_type::int32> {
        static const size_t size = sizeof(std::int32_t);
        typedef std::int32_t type;
    };

    template<> struct scalar_type_traits<scalar_type::int64> {
        static const size_t size = sizeof(std::int64_t);
        typedef std::int64_t type;
    };


    /// <summary>
    /// A traits class for inferring the
    /// <see cref="trrojan::stream::scalar_type" /> type from a C++ type.
    /// </summary>
    template<class T> struct scalar_reverse_traits { };

    template<> struct scalar_reverse_traits<float> {
        static const size_t size = sizeof(float);
        static const scalar_type type = scalar_type::float32;
    };

    template<> struct scalar_reverse_traits<double> {
        static const size_t size = sizeof(double);
        static const scalar_type type = scalar_type::float64;
    };

    template<> struct scalar_reverse_traits<std::int32_t> {
        static const size_t size = sizeof(std::int32_t);
        static const scalar_type type = scalar_type::int32;
    };

    template<> struct scalar_reverse_traits<std::int64_t> {
        static const size_t size = sizeof(std::int64_t);
        static const scalar_type type = scalar_type::int64;
    };
}
}
