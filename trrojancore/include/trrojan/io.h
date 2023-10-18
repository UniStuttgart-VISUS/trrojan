// <copyright file="io.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cinttypes>
#include <cstdio>
#include <string>
#include <vector>
#include <stack>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else /* _WIN32 */
#include <dirent.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif /* _WIN32 */

#include "trrojan/export.h"
#include "trrojan/text.h"


namespace trrojan {

    /// <summary>
    /// The native representation of a file system entry as processed
    /// by <see cref="get_file_system_entries" />.
    /// </summary>
#ifdef _WIN32
    typedef WIN32_FIND_DATAA file_system_entry;
#else /* _WIN32 */
    typedef struct dirent file_system_entry;
#endif /* _WIN32 */

    /// <summary>
    /// A predicate which determines whether a directory entry has a specific
    /// extension.
    /// </summary>
    struct TRROJANCORE_API has_extension {

        std::string extension;

        inline has_extension(const std::string& ext) : extension(ext) { }

        inline bool operator ()(const file_system_entry& fd) {
#ifdef _WIN32
            return trrojan::ends_with(std::string(fd.cFileName),
                this->extension);
#else /* _WIN32 */
            return trrojan::ends_with(std::string(fd.d_name),
                this->extension);
#endif /* _WIN32 */
        }
    };

    /// <summary>
    /// A predicate which determines whether a file system entry is a directory.
    /// </summary>
    struct TRROJANCORE_API is_directory {

        inline bool operator ()(const file_system_entry& fd) {
#ifdef _WIN32
            return ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
#else /* _WIN32 */
            return (fd.d_type == DT_DIR);
#endif /* _WIN32 */
        }
    };

    /// <summary>
    /// Combines <paramref name="paths" /> with
    /// <see cref="trrojan::directory_separator_char" />.
    /// </summary>
    template<class... P>
    std::string combine_path(std::string path, P&&... paths);

    /// <summary>
    /// Ensures that the given path ends with a
    /// <see cref="directory_separator_char" />.
    /// </summary>
    /// <param name="path">The path to terminate. It is safe to pass
    /// <c>nullptr</c>, which will be interpreted as an empty string.</param>
    /// <returns>The terminated string.</returns>
    std::string TRROJANCORE_API ensure_directory_end(const char *path);

    /// <summary>
    /// Ensures that the given path ends with a
    /// <see cref="directory_separator_char" />.
    /// </summary>
    /// <param name="path">The path to terminate.</param>
    /// <returns>The terminated string.</returns>
    std::string TRROJANCORE_API ensure_directory_end(const std::string& path);

    /// <summary>
    /// Gets the file name extension of <paramref name="path" /> including the
    /// leading <see cref="trrojan::extension_separator_char" />.
    /// </summary>
    std::string TRROJANCORE_API get_extension(const std::string& path);

    /// <summary>
    /// Enumerates all file system entries (files and directories) in
    /// <paramref name="path" /> that match the given pattern and the given
    /// predicate.
    /// </summary>
    /// <param name="oit">An output iterator that will receive the paths.
    /// </param>
    /// <param name="path">The root path to be searched.</param>
    /// <param name="isRecursive">Determines whether directories will be
    /// searched recursively.</param>
    /// <param name="predicate">An additional predicate working on the native
    /// search results that enables filtering of results.</param>
    /// <exception cref="std::system_error">In case the operation failed.
    /// </exception>
    template<class I, class P>
    void get_file_system_entries(I oit, const std::string& path,
        const bool isRecursive, P predicate);

    /// <summary>
    /// Enumerates all file system entries (files and directories) in
    /// <paramref name="path" />.
    template<class I>
    inline void get_file_system_entries(I oit, const std::string& path,
            const bool isRecursive) {
        get_file_system_entries(oit, path, isRecursive,
            [](file_system_entry&) { return true; });
    }

    /// <summary>
    /// Returns the base name of the file without the path.
    /// </summary>
    /// <param name="path">The path to extract the name from.</param>
    /// <param name="with_extension">If <c>true</c>, return the file name
    /// with the extension, otherwise remove all characters starting from
    /// the last <see cref="extension_separator_char" />.</param>
    /// <retruns>The extracted file name.</retruns>
    std::string TRROJANCORE_API get_file_name(const std::string& path,
        const bool with_extension = true);

    /// <summary>
    /// Gets the path to the local storage folder of an UWP app or the
    /// application folder on all other platforms.
    /// </summary>
    /// <returns>The local storage folder.</returns>
    std::string TRROJANCORE_API get_local_storage_folder(void);

    /// <summary>
    /// Returns the path of the file. This does not include the file name.
    /// </summary>
    /// <param name="file_path">The file path to extract the path from.</param>
    /// <retruns>The extracted path.</retruns>
    std::string TRROJANCORE_API get_path(const std::string& file_path);


    /// <summary>
    /// Joins two paths with a <see cref="directory_separator_char" />.
    /// </summary>
    /// <param name="lhs">The left-hand side operand. It is safe to pass
    /// <see cref="nullptr" />, which will be interpreted as an empty string.
    /// </param>
    /// <param name="rhs">The right-hand side operand. It is safe to pass
    /// <see cref="nullptr" />, which will be interpreted as an empty string.
    /// </param>
    /// <returns>The joined path.</returns>
    std::string TRROJANCORE_API join_path(const char *lhs, const char *rhs);

    /// <summary>
    /// Joins two paths with a <see cref="directory_separator_char" />.
    /// </summary>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    inline std::string join_path(const std::string& lhs,
            const std::string& rhs) {
        return join_path(lhs.c_str(), rhs.c_str());
    }

    /// <summary>
    /// Joins two or more paths with <see cref="directory_separator_char" />s.
    /// </summary>
    /// <typeparam name="TRights"></typeparam>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    template<class... TRights>
    inline std::string join_path(const std::string& lhs, TRights&&... rhs) {
        return join_path(lhs, join_path(std::forward<TRights>(rhs)...));
    }

    /// <summary>
    /// Recursion stop for variadic <see cref="join_path" />.
    /// </summary>
    /// <param name="lhs"></param>
    /// <returns></returns>
    inline const std::string& join_path(const std::string& lhs) {
        return lhs;
    }

    /// <summary>
    /// Recursion stop for variadic <see cref="join_path" />.
    /// </summary>
    /// <returns></returns>
    inline std::string join_path(void) {
        return std::string("");
    }

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    std::vector<std::uint8_t> TRROJANCORE_API read_binary_file(
        const char *path);

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    inline std::vector<std::uint8_t> TRROJANCORE_API read_binary_file(
            const std::string& path) {
        return read_binary_file(path.c_str());
    }

    /// <summary>
    /// Read a whole text file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    std::string TRROJANCORE_API read_text_file(const char *path);

    /// <summary>
    /// Read a whole text file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    inline std::string TRROJANCORE_API read_text_file(const std::string& path) {
        return read_text_file(path.c_str());
    }

    /// <summary>
    /// Specifies the alternative directory separator character if the
    /// platform uses one. Otherwise, this value is equivalent to
    /// <see cref="trrojan::directory_separator_char" />.
    /// <summary>
    extern const char TRROJANCORE_API alt_directory_separator_char;

    /// <summary>
    /// The name of the current directory (usually ".").
    /// </summary>
    extern const std::string TRROJANCORE_API current_directory_name;

    /// <summary>
    /// Specifies the directory separator character.
    /// </summary>
    extern const char TRROJANCORE_API directory_separator_char;

    /// <summary>
    /// The character used to introduce an extension ('.').
    /// </summary>
    extern const char TRROJANCORE_API extension_separator_char;

    /// <summary>
    /// The name of the parent directory (usually "..").
    /// </summary>
    extern const std::string TRROJANCORE_API parent_directory_name;

    /// <summary>
    /// Specifies the separator character for the path environment variable.
    /// </summary>
    extern const char TRROJANCORE_API path_separator_char;

    /// <summary>
    /// Specifies the separator for a volume.
    /// </summary>
    extern const char TRROJANCORE_API volume_separator_char;

}

#include "trrojan/io.inl"
