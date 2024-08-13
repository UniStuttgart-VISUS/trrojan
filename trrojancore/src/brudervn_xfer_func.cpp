// <copyright file="brudervn_xfer_func.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/brudervn_xfer_func.h"

#include <fstream>
#include <sstream>
#include <stdexcept>


/*
 * trrojan::load_brudervn_xfer_func
 */
std::vector<std::uint8_t> trrojan::load_brudervn_xfer_func(
        const std::string& path) {
    std::vector<std::uint8_t> retval;
    std::uint32_t value;

    std::ifstream file(path, std::ios::in);
    if (file.is_open()) {
        retval.reserve(256);

        while (file >> value) {
            retval.push_back(static_cast<std::uint8_t>(value));
        }

    } else {
        std::stringstream msg;
        msg << "Failed to open \"" << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
