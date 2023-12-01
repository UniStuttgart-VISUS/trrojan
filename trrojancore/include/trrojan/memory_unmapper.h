// <copyright file="memory_unmapper.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once


#if defined(_WIN32)
#include <Windows.h>
#endif /* defined(_WIN32) */


namespace trrojan {

    /// <summary>
    /// A deleter for <see cref="std::unique_ptr" /> that unmaps memory mapped
    /// views of a file.
    /// </summary>
    struct memory_unmapper final {

        inline void operator ()(void *ptr) const noexcept {
#if defined(_WIN32)
            ::UnmapViewOfFile(ptr);
#endif /* defined(_WIN32) */
        }

    };


} /* namespace trrojan */
