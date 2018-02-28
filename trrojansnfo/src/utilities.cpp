/// <copyright file="utilities.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "utilities.h"

#include <fstream>
#include <sstream>


/*
 * trrojan::sysinfo::detail::read_all_bytes
 */
std::vector<std::uint8_t> trrojan::sysinfo::detail::read_all_bytes(
        const char *path) {
    if (path == nullptr) {
        throw std::invalid_argument("'path' must not be nullptr.");
    }

    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file) {
        std::stringstream msg;
        msg << "Failed to open \"" << path << "\"" << std::ends;
        throw std::runtime_error(msg.str());
    }

    std::vector<std::uint8_t> retval(file.tellg());
    file.seekg(0, std::ios::beg);

    if (!file.read(reinterpret_cast<char *>(retval.data()), retval.size())) {
        std::stringstream msg;
        msg << "Failed to read all " << retval.size()
            << " bytes from \"" << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return retval;
}
