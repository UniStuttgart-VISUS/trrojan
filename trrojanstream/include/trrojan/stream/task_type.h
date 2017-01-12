/// <copyright file="task_type.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/stream/export.h"


namespace trrojan {
namespace stream {

    /// <summary>
    /// Possible types of streaming tasks.
    /// </summary>
    enum class TRROJANSTREAM_API task_type {

        /// <summary>
        /// Add numbers from two arrays and store the result in a third one.
        /// </summary>
        add,

        /// <summary>
        /// Copy from one array to another.
        /// </summary>
        copy,

        /// <summary>
        /// Multiply numbers from an array with a scalar value and store the
        /// result in another array.
        /// </summary>
        scale,

        /// <summary>
        /// Multiply numbers from an array with a scalar, add values from
        /// another array and store the result in a third one.
        /// </summary>
        triad
    };


    template<task_type...> struct task_type_list_t { };

    typedef task_type_list_t<task_type::add, task_type::copy,
        task_type::scale, task_type::triad> task_type_list;
}
}
