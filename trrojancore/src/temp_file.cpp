// <copyright file="temp_file.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/temp_file.h"

#include <array>
#include <cassert>
#include <cerrno>
#include <memory>
#include <system_error>

#if defined(_WIN32)
#include <Windows.h>
#else /* defined(_WIN32) */
#include <fcntl.h>
#include <unistd.h>
#endif /* defined(_WIN32) */

#include "trrojan/io.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::temp_file::create
 */
trrojan::temp_file trrojan::temp_file::create(const char *prefix) {
    return create(nullptr, prefix);
}


/*
 * trrojan::temp_file::create
 */
trrojan::temp_file trrojan::temp_file::create(const char *folder,
        const char *prefix) {
#if defined(_WIN32)
    std::array<char, MAX_PATH + 1> buffer;

    if (folder != nullptr) {
        ::strncpy(buffer.data(), folder, buffer.size());
    } else {
        // Note: The path can never exceed the constant length above 
        // according to
        // https://learn.microsoft.com/de-de/windows/win32/api/fileapi/nf-fileapi-gettemppatha
        if (!::GetTempPathA(static_cast<DWORD>(buffer.size()), buffer.data())) {
            throw std::system_error(::GetLastError(), std::system_category());
        }
    }

    auto p = (prefix != nullptr) ? prefix : "trrojan";

    if (!::GetTempFileNameA(buffer.data(), p, 0, buffer.data())) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return temp_file(buffer.data());

#else /* defined(_WIN32) */
    std::string retval = (folder != nullptr) ? folder : get_temp_folder();

    if (prefix != nullptr) {
        retval = combine_path(retval, prefix);
    } else {
        retval = combine_path(retval, prefix);
    }

    retval += "XXXXXX";

    auto fd = ::mkstemp(retval.data());
    if (fd == -1) {
        throw std::system_error(errno, std::system_category());
    }

    ::close(fd);

    return temp_file(retval);
#endif /* defined(_WIN32) */
}


/*
 * trrojan::temp_file::temp_file
 */
trrojan::temp_file::temp_file(temp_file&& rhs) noexcept
        : _path(std::move(rhs._path)) {
    assert(!rhs);
}


/*
 * trrojan::temp_file::~temp_file
 */
trrojan::temp_file::~temp_file(void) noexcept {
    this->release();
}


/*
 * trrojan::temp_file::release
 */
void trrojan::temp_file::release(void) noexcept {
    if (*this) {
        trrojan::log::instance().write_line(trrojan::log_level::debug,
            "Releasing temporary file \"{0}\" ...", this->_path);
#if defined(_WIN32)
        if (!::DeleteFileA(this->_path.c_str())) {
#else /* defined(_WIN32) */
        if (::unlink(this->_path.c_str()) < 0) {
#endif /* defined(_WIN32) */
            trrojan::log::instance().write_line(trrojan::log_level::warning,
                "Failed to release temporary file \"{0}\" ...", this->_path);
        }

        this->_path.clear();
    }
    assert(!*this);
}


/*
 * trrojan::temp_file::operator =
 */
trrojan::temp_file& trrojan::temp_file::operator =(temp_file&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        this->release();
        this->_path = std::move(rhs._path);
        assert(!rhs);
    }

    return *this;
}


/*
 * trrojan::temp_file::operator std::wstring
 */
trrojan::temp_file::operator std::wstring(void) const {
    return from_utf8(this->_path);
}


/*
 * trrojan::temp_file::temp_file
 */
trrojan::temp_file::temp_file(const std::string& path) : _path(path) { }
