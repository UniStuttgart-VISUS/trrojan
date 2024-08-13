// <copyright file="constants.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <climits>
#include <cmath>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Provides some useful constants.
    /// </summary>
    template<class T> class constants {

    public:

        /// <summary>
        /// Specifies the type to retrieve the constant as.
        /// </summary>
        typedef T type;

        /// <summary>
        /// Specifies the number of bits in a byte.
        /// </summary>
        static constexpr type bits_per_byte = static_cast<T>(CHAR_BIT);

        /// <summary>
        /// Specifies the number of bytes in a Gigabyte.
        /// </summary>
        static constexpr type bytes_per_gigabyte
            = static_cast<T>(1024 * 1024 * 1024);

        /// <summary>
        /// Specifies the number of bytes in a kilobyte.
        /// </summary>
        static constexpr type bytes_per_kilobyte = static_cast<T>(1024);

        /// <summary>
        /// Specifies the number of bytes in a Megabyte.
        /// <summary>
        static constexpr type bytes_per_megabyte = static_cast<T>(1024 * 1024);

        /// <summary>
        /// Specifies the number milliseconds per second.
        /// </summary>
        static constexpr type millis_per_second = static_cast<T>(1000);

        /// <summary>
        // The PI constant.
        /// </summary>
        static constexpr type pi = static_cast<T>(3.14159265358979323846);

        /// <summary>
        /// Specifies the number seconds per hour.
        /// </summary>
        static constexpr type seconds_per_hour = static_cast<T>(60 * 60);

        /// <summary>
        /// Specifies the number seconds per minute.
        /// </summary>
        static constexpr type seconds_per_minute = static_cast<T>(60);

        constants(void) = delete;

        ~constants(void) = delete;
    };
}
