// <copyright file="power_compatibility_sink.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2026 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE file for more information.
// </copyright>
// <author>Christoph Müller</author>

#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
#include "trrojan/power_compatibility_sink.h"


/*
 * trrojan::power_compatibility_sink::power_compatibility_sink
 */
trrojan::power_compatibility_sink::power_compatibility_sink(
        _In_z_ const char *path)
        : _delimiter(';'), _first(true) {
    this->_stream.open(path, std::ios::trunc);
    if (!this->_stream.is_open()) {
        throw std::invalid_argument("The output stream could not be opened.");
    }
}

#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
