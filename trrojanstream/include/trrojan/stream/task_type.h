/// <copyright file="task_type.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/enum_dispatch_list.h"

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


    /// <summary>
    /// A traits class for parsing task types.
    /// </summary>
    template<task_type S> struct task_type_traits { };

#define __TRROJANCORE_DECL_TASK_TYPE_TRAITS(t, a)                              \
    template<> struct task_type_traits<task_type::t> {                         \
        static const size_t memory_accesses = a;                               \
        static inline const std::string& name(void) {                          \
            static const std::string retval(#t);                               \
            return retval;                                                     \
        }                                                                      \
    }

    __TRROJANCORE_DECL_TASK_TYPE_TRAITS(add, 3);
    __TRROJANCORE_DECL_TASK_TYPE_TRAITS(copy, 2);
    __TRROJANCORE_DECL_TASK_TYPE_TRAITS(scale, 2);
    __TRROJANCORE_DECL_TASK_TYPE_TRAITS(triad, 3);

#undef __TRROJANCORE_DECL_TASK_TYPE_TRAITS


    template<task_type... V>
    using task_type_list_t = enum_dispatch_list<task_type, V...>;

    typedef task_type_list_t<task_type::add, task_type::copy,
        task_type::scale, task_type::triad> task_type_list;
}
}
