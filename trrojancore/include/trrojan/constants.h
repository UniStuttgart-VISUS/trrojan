/// <copyright file="constants.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

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
        static const type bits_per_byte;

        /// <summary>
        /// Specifies the number of bytes in a Gigabyte.
        /// </summary>
        static const type bytes_per_gigabyte;

        /// <summary>
        /// Specifies the number of bytes in a kilobyte.
        /// </summary>
        static const type bytes_per_kilobyte;

        /// <summary>
        /// Specifies the number of bytes in a Megabyte.
        /// <summary>
        static const type bytes_per_megabyte;

        /// <summary>
        /// Specifies the number milliseconds per second.
        /// </summary>
        static const type millis_per_second;

        /// <summary>
        // The PI constant.
        /// </summary>
        static const type pi;

        /// <summary>
        /// Specifies the number seconds per hour.
        /// </summary>
        static const type seconds_per_hour;

        /// <summary>
        /// Specifies the number seconds per minute.
        /// </summary>
        static const type seconds_per_minute;

        constants(void) = delete;

        ~constants(void) = delete;
    };
}

#include "trrojan/constants.inl"
