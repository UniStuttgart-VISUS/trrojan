/// <copyright file="io.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/io.h"

#include <fstream>
#include <sstream>


/*
 * trrojan::read_binary_file
 */
std::vector<char> trrojan::read_binary_file(const char *path) {
    if (path == nullptr) {
        throw std::invalid_argument("'path' must not be nullptr.");
    }

    std::fstream file(path, std::ios::binary | std::ios::ate);
    std::vector<char> retval(file.tellg());

    file.seekg(0, std::ios::beg);
    if (file.read(retval.data(), retval.size())) {
        std::stringstream msg;
        msg << "Failed reading all data from \"" << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return std::move(retval);
}
