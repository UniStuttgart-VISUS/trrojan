// <copyright file="io.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2024 Visualisierungsinstitut der Universität Stuttgart.
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
#include <system_error>

#if defined(TRROJAN_FOR_UWP)
#include <winrt/windows.foundation.h>
#include <winrt/windows.applicationModel.core.h>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.streams.h>
#endif /* defined(TRROJAN_FOR_UWP) */

#include "trrojan/com_error_category.h"
#include "trrojan/executive.h"
#include "trrojan/on_exit.h"


/*
 * trrojan::append_copies_to_file
 */
void trrojan::append_copies_to_file(const std::string& path,
        const std::size_t cnt) {
    std::fstream file(path, std::ios::in | std::ios::out | std::ios::binary
        | std::ios::ate);
    if (!file) {
        std::stringstream msg;
        msg << "Failed to open \"" << path << "\"" << std::ends;
        throw std::runtime_error(msg.str());
    }

    std::vector<char> buffer(file.tellg());
    file.seekg(0, std::ios::beg);

    if (!file.read(buffer.data(), buffer.size())) {
        std::stringstream msg;
        msg << "Failed to read the current content from \""
            << path << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }

    for (std::size_t i = 0; i < cnt; ++i) {
        if (!file.write(buffer.data(), buffer.size())) {
            std::stringstream msg;
            msg << "Failed to append a copy of the data to \""
                << path << "\"." << std::ends;
            throw std::runtime_error(msg.str());
        }
    }
}


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
 * trrojan::get_temp_folder
 */
std::string trrojan::get_temp_folder(void) {
#if defined(_WIN32)
    std::array<char, MAX_PATH + 1> retval;

    if (!::GetTempPathA(static_cast<DWORD>(retval.size()), retval.data())) {
        throw std::system_error(::GetLastError(), std::system_category());
    }

    return retval.data();

#else /* defined(_WIN32) */
    auto retval = std::getenv("TMPDIR");

    if (retval == nullptr) {
        retval = std::getenv("TEMPDIR");
    }

    if (retval == nullptr) {
        retval = std::getenv("TMP");
    }

    if (retval == nullptr) {
        retval = std::getenv("TEMP");
    }

    if (retval == nullptr) {
        retval = "/tmp";
    }

    return retval;
#endif /* defined(_WIN32) */
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


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::read_text_file
 */
std::string trrojan::read_text_file(
        const winrt::Windows::Storage::StorageFile file) {
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Windows::Storage::Streams;

    auto evt = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (evt == NULL) {
        throw std::system_error(::GetLastError(), std::system_category());
    }
    on_exit([evt](void) { ::CloseHandle(evt); });

    std::exception_ptr error;
    std::string retval;

    auto on_error = [&error, evt](const AsyncStatus, const winrt::hresult hr) {
        try {
            winrt::throw_hresult(hr);
        } catch (...) {
            error = std::current_exception();
        }
        ::SetEvent(evt);
    };

    on_completed(file.OpenReadAsync(),
            [&retval, evt, &on_error](IRandomAccessStream s) {
        try {
            auto size = static_cast<std::uint32_t>(s.Size());
            auto buffer = Buffer(size);

            on_completed(s.ReadAsync(buffer, size, InputStreamOptions::None),
                    [&retval, evt](IBuffer b) {
                on_exit([evt](void) { ::SetEvent(evt); });
                auto d = reinterpret_cast<const char *>(b.data());
                retval = std::string(d, d + b.Length());
            }, on_error);

        } catch (...) {
            ::SetEvent(evt);
            throw;
        }
    }, on_error);

    // F*#$%! you, STA thread ...
    ::WaitForSingleObject(evt, INFINITE);

    if (error) {
        std::rethrow_exception(error);
    } else {
        return retval;
    }
}
#endif /* defined(TRROJAN_FOR_UWP) */


#if defined(_WIN32)
/*
 * trrojan::write_all_bytes
 */
void trrojan::write_all_bytes(HANDLE handle, const void *data,
        const std::size_t cnt) {
    auto cur = static_cast<const std::uint8_t *>(data);
    auto rem = cnt;

    while (rem > 0) {
        DWORD written = 0;
        if (!::WriteFile(handle, cur, rem, &written, nullptr)) {
            throw std::system_error(::GetLastError(), std::system_category());
        }

        assert(written <= rem);
        cur += written;
        rem -= written;
    }
}
#endif defined(_WIN32)


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
