// <copyright file="io.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Valentin Bruder</author>

#include "trrojan/io.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>


/*
 * trrojan::read_binary_file
 */
std::vector<std::uint8_t> TRROJANCORE_API trrojan::read_binary_file(
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


/*
 * trrojan::read_text_file
 */
std::string TRROJANCORE_API trrojan::read_text_file(const char *path) {
    if (path == nullptr) {
        throw std::invalid_argument("'path' must not be nullptr.");
    }

    std::ifstream file(path, std::ios::in | std::ios::ate);
    if (file) {
        std::string retval;
        retval.reserve(file.tellg());
        file.seekg(0, std::ios::beg);

        retval.assign((std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>());

        return retval;

    } else {
        std::stringstream msg;
        msg << "Failed to open \"" << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }
}


/*
 * trrojan::get_extension
 */
std::string trrojan::get_extension(const std::string& path) {
    auto idx = path.find_last_of(extension_separator_char);
    if (idx == std::string::npos) {
        return std::string();
    } else {
        return path.substr(idx);
    }
}


/*
 * trrojan::get_file_name
 */
std::string TRROJANCORE_API trrojan::get_file_name(const std::string& path,
        const bool with_extension) {
    std::string retval = path.substr(path.find_last_of(directory_separator_char) + 1);
    if (!with_extension) {
        retval = retval.substr(0, retval.find_last_of("."));
    }
    return retval;
}


/*
 * trrojan::get_path
 */
std::string TRROJANCORE_API trrojan::get_path(const std::string& file_path) {
    return file_path.substr(0, file_path.find_last_of(directory_separator_char));
}


/*
 * trrojan::alt_directory_separator_char
 */
const char TRROJANCORE_API trrojan::alt_directory_separator_char = '/';


/*
 * trrojan::current_directory_name
 */
const std::string TRROJANCORE_API trrojan::current_directory_name(".");


/*
 * trrojan::directory_separator_char
 */
const char TRROJANCORE_API trrojan::directory_separator_char
#ifdef _WIN32
    = '\\';
#else /* _WIN32 */
    = '/';
#endif /* _WIN32 */


/*
 * trrojan::extension_separator_char
 */
const char TRROJANCORE_API trrojan::extension_separator_char = '.';


/*
 * trrojan::parent_directory_name
 */
const std::string TRROJANCORE_API trrojan::parent_directory_name("..");


/*
 * trrojan::path_separator_char
 */
const char TRROJANCORE_API trrojan::path_separator_char
#if defined(_WIN32)
    = ';';
#else /* defined(_WIN32) */
    = ':';
#endif /* defined(_WIN32) */


/*
 * trrojan::volume_separator_char
 */
const char TRROJANCORE_API trrojan::volume_separator_char
#if defined(_WIN32)
    = ':';
#else /* defined(_WIN32) */
    = '/';
#endif /* defined(_WIN32) */
