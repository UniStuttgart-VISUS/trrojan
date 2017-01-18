/// <copyright file="io.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdio>
#include <string>
#include <vector>
#include <stack>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
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
    struct has_extension {

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
    /// Combines <paramref name="paths" /> with
    /// <see cref="trrojan::directory_separator_char" />.
    /// </summary>
    template<class... P> std::string TRROJANCORE_API combine_path(
        std::string path, P&&... paths);

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
    void TRROJANCORE_API get_file_system_entries(I oit,
        const std::string& path, const bool isRecursive, P predicate);

    /// <summary>
    /// Enumerates all file system entries (files and directories) in
    /// <paramref name="path" />.
    template<class I>
    inline void TRROJANCORE_API get_file_system_entries(I oit,
            const std::string& path, const bool isRecursive) {
        get_file_system_entries(oit, path, isRecursive,
            [](file_system_entry&) { return true; });
    }

    /// <summary>
    /// Returns the base name of the file without the path.
    /// </summary>
    /// <param name="path">The path to extrct the name from.</param>
    /// <param name="with_extension">If <c>true</c>, return the file name with extension,
    /// otherwise remove all characters starting with the last '.'.</param>
    /// <retruns>The extracted file name.</returns>
    std::string TRROJANCORE_API get_file_name(const std::string &path,
                                              const bool with_extension = true);

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    std::vector<char> TRROJANCORE_API read_binary_file(const char *path);

    /// <summary>
    /// Read a whole binary file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    inline std::vector<char> TRROJANCORE_API read_binary_file(
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
    extern const std::string  TRROJANCORE_API parent_directory_name;

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
