/// <copyright file="utilities.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "utilities.h"

#include <array>
#include <cerrno>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <system_error>



/*
 * trrojan::sysinfo::detail::invoke
 */
std::string trrojan::sysinfo::detail::invoke(const char *cmd) {
    if (cmd == nullptr) {
        throw std::invalid_argument("'cmd' must not be nullptr.");
    }

    std::array<char, 128> buffer;
    std::string retval;

#if defined(_WIN32)
    std::shared_ptr<FILE> fp(::_popen(cmd, "r"), ::_pclose);
#else /* defined(_WIN32) */
    std::shared_ptr<FILE> fp(::_popen(cmd, "r"), ::_pclose);
#endif /* defined(_WIN32) */

    if (fp == nullptr) {
        throw std::system_error(errno, std::system_category(),
            "Failed to open a pipe for the command.");
    }

    while (!::feof(fp.get())) {
        if (::fgets(buffer.data(), static_cast<int>(buffer.size()),
                fp.get()) != nullptr) {
            retval += buffer.data();
        }
    }

    return retval;
}


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


#if defined(_WIN32)
/*
 * trrojan::sysinfo::detail::read_reg_value
 */
LSTATUS trrojan::sysinfo::detail::read_reg_value(std::vector<std::uint8_t>& dst,
        DWORD& type, HKEY key, const char *name) {
    LSTATUS retval = ERROR_SUCCESS;
    DWORD size = 0;

    // Determine required size of 'dst'.
    retval = ::RegQueryValueExA(key, name, nullptr, &type, nullptr, &size);

    // Get the value.
    if ((retval == ERROR_SUCCESS) || (retval == ERROR_MORE_DATA)) {
        dst.resize(size);
        retval = ::RegQueryValueExA(key, name, nullptr, &type, dst.data(),
            &size);
    }

    return retval;
}
#endif /* defined (_WIN32) */
