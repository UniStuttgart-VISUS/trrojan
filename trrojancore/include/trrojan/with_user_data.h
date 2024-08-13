// <copyright file="with_user_data.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <type_traits>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// A base class that allows for attaching user data to a derived type.
    /// </summary>
    /// <typeparam name="TUserData">The type of the user data to be attached,
    /// which can be <c>void</c> for attaching no user data.</typeparam>
    template<class TUserData> struct with_user_data {

        /// <summary>
        /// The type of the user data to be attached.
        /// </summary>
        typedef TUserData user_data_type;

        /// <summary>
        /// The attached user data.
        /// </summary>
        user_data_type user_data;
    };


    /// <summary>
    /// Specialisation for no user data.
    /// </summary>
    template<> struct TRROJANCORE_API with_user_data<void> {

        /// <summary>
        /// The type of the user data to be attached.
        /// </summary>
        typedef void user_data_type;
    };

} /* namespace trrojan */
