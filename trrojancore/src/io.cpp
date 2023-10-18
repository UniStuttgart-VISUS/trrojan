// <copyright file="io.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>
// <author>Valentin Bruder</author>
// <author>Michael Becher</author>

#include "trrojan/io.h"

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <streambuf>

#if defined(TRROJAN_FOR_UWP)
#include <winrt/windows.applicationModel.core.h>
#include <winrt/windows.storage.h>
#endif /* defined(TRROJAN_FOR_UWP) */

#include "trrojan/executive.h"


/*
 * trrojan::ensure_directory_end
 */
std::string trrojan::ensure_directory_end(const char *path) {
    if ((path == nullptr) || (*path == 0)) {
        return std::string(1, directory_separator_char);
    } else {
        return ensure_directory_end(std::string(path));
    }
}


/*
 * trrojan::ensure_directory_end
 */
std::string trrojan::ensure_directory_end(const std::string& path) {
    if (path.empty()) {
        return std::string(1, directory_separator_char);
    } else if (path.back() != directory_separator_char) {
        return path + directory_separator_char;
    } else {
        return path;
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
std::string trrojan::get_file_name(const std::string& path,
        const bool with_extension) {
    const auto begin_file = path.find_last_of(directory_separator_char) + 1;
    std::string retval = path.substr(begin_file);

    if (!with_extension) {
        const auto end_file = retval.find_last_of(".");
        retval = retval.substr(0, end_file);
    }

    return retval;
}


/*
 * trrojan::get_local_storage_folder
 */
std::string trrojan::get_local_storage_folder(void) {
#if defined(TRROJAN_FOR_UWP)
    using namespace winrt::Windows::Storage;
    auto folder = ApplicationData::Current().LocalFolder();
    auto retval = ensure_directory_end(winrt::to_string(folder.Path()));
    return retval;

#else /* defined(TRROJAN_FOR_UWP) */
    return executive::executable_directory();
#endif /* defined(TRROJAN_FOR_UWP) */
}


/*
 * trrojan::get_path
 */
std::string TRROJANCORE_API trrojan::get_path(const std::string& file_path) {
    const auto begin_file = file_path.find_last_of(directory_separator_char);
    return file_path.substr(0, begin_file);
}


/*
 * trrojan::join_path
 */
std::string trrojan::join_path(const char *lhs, const char *rhs) {
    if ((lhs == nullptr) && (rhs == nullptr)) {
        return std::string();

    } else if (lhs == nullptr) {
        return rhs;

    } else if (rhs == nullptr) {
        return lhs;

    } else {
        std::string retval(lhs);

        if (retval.empty() || (retval.back() != directory_separator_char)) {
            retval += directory_separator_char;
        }

        retval += rhs;

        return retval;
    }
}


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
