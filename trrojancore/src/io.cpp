/// <copyright file="io.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/io.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>


/*
 * trrojan::read_binary_file
 */
std::vector<char> trrojan::read_binary_file(const char *path) {
    if (path == nullptr) {
        throw std::invalid_argument("'path' must not be nullptr.");
    }

    std::fstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file) {
        std::stringstream msg;
        msg << "Failed to open \"" << path << "\"" << std::ends;
        throw std::runtime_error(msg.str());
    }

    std::vector<char> retval(file.tellg());
    file.seekg(0, std::ios::beg);

    if (!file.read(retval.data(), retval.size())) {
        std::stringstream msg;
        msg << "Failed to read all " << retval.size()
            << " bytes from \"" << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }

    return std::move(retval);
}


/*
 * trrojan::read_text_file
 */
 std::string trrojan::read_text_file(const char *path) {
     if (path == nullptr) {
         throw std::invalid_argument("'path' must not be nullptr.");
     }

     std::fstream file(path, std::ios::in | std::ios::ate);
     if (!file) {
         std::stringstream msg;
         msg << "Failed to open \"" << path << "\"" << std::ends;
         throw std::runtime_error(msg.str());
     }

     std::string retval;
     retval.reserve(file.tellg());
     file.seekg(0, std::ios::beg);

    retval.assign((std::istreambuf_iterator<char>(file)),
         std::istreambuf_iterator<char>());

    return retval;
 }
