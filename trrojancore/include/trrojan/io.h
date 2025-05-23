﻿// <copyright file="io.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <cinttypes>
#include <cstdio>
#include <string>
#include <vector>
#include <stack>
#include <stdexcept>
#include <system_error>

#if defined(_WIN32)
#include <Windows.h>
#include <direct.h>
#else /* defined(_WIN32) */
#include <dirent.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif /* defined(_WIN32) */

#if defined(TRROJAN_FOR_UWP)
#include <winrt/windows.applicationmodel.core.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.storage.h>
#include <winrt/windows.storage.pickers.h>
#endif /* defined(TRROJAN_FOR_UWP) */

#include "trrojan/export.h"
#include "trrojan/temp_file.h"
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
    /// Appends <paraemref name="cnt" /> copies of the current content of
    /// the file at <paramref name="path" /> to the file.
    /// </summary>
    /// <param name="path"></param>
    /// <param name="cnt"></param>
    /// <returns></returns>
    void TRROJANCORE_API append_copies_to_file(const std::string& path,
        const std::size_t cnt);

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
    /// Gets the path of the temporary folder.
    /// </summary>
    /// <returns>Get the path of the temporary folder as configured on the
    /// system or a fallback path.</returns>
    std::string TRROJANCORE_API get_temp_folder(void);

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Passes the result of the given operation to the given callback once the
    /// operation completed.
    /// </summary>
    /// <typeparam name="TResult"></typeparam>
    /// <typeparam name="TSuccess"></typeparam>
    /// <typeparam name="TFailure"></typeparam>
    /// <param name="operation"></param>
    /// <param name="success_action"></param>
    /// <param name="failure_action"></param>
    template<class TResult, class TSuccess, class TFailure>
    void on_completed(
            winrt::Windows::Foundation::IAsyncOperation<TResult> operation,
            TSuccess&& success_action,
            TFailure&& failure_action) {
        using namespace winrt::Windows::Foundation;
        assert(operation);
        operation.Completed([success_action, failure_action](
                IAsyncOperation<TResult> operation,
                const AsyncStatus status) {
            if (status == AsyncStatus::Completed) {
                success_action(operation.get());
            } else {
                failure_action(status, operation.ErrorCode());
            }
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Passes the result of the given operation to the given callback once the
    /// operation completed.
    /// </summary>
    /// <typeparam name="TSuccess"></typeparam>
    /// <typeparam name="TFailure"></typeparam>
    /// <param name="operation"></param>
    /// <param name="success_action"></param>
    /// <param name="failure_action"></param>
    template<class TSuccess, class TFailure>
    void on_completed(winrt::Windows::Foundation::IAsyncAction operation,
            TSuccess&& success_action,
            TFailure&& failure_action) {
        using namespace winrt::Windows::Foundation;
        assert(operation);
        operation.Completed([success_action, failure_action](
                IAsyncAction operation, const AsyncStatus status) {
            if (status == AsyncStatus::Completed) {
                success_action();
            } else {
                failure_action(status, operation.ErrorCode());
            }
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Passes the result of the given operation to the given callback once the
    /// operation completed.
    /// </summary>
    /// <typeparam name="TResult"></typeparam>
    /// <typeparam name="TProgress"></typeparam>
    /// <typeparam name="TSuccess"></typeparam>
    /// <typeparam name="TFailure"></typeparam>
    /// <param name="operation"></param>
    /// <param name="action"></param>
    template<class TResult, class TProgress, class TSuccess, class TFailure>
    void on_completed(winrt::Windows::Foundation::IAsyncOperationWithProgress<
            TResult, TProgress> operation,
            TSuccess&& success_action,
            TFailure&& failure_action) {
        using namespace winrt::Windows::Foundation;
        assert(operation);
        operation.Completed([success_action, failure_action](
                IAsyncOperationWithProgress<TResult, TProgress> operation,
                const AsyncStatus status) {
            if (status == AsyncStatus::Completed) {
                success_action(operation.get());
            } else {
                failure_action(status, operation.ErrorCode());
            }
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks a file for writing using the given <paramref name="picker" />
    /// and calls the given action with the selected
    /// <see cref="winrt::Windows::Storage::StorageFile" />.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a single <see cref="winrt::Windows::Storage::StorageFile" />
    /// parameter.</typeparam>
    /// <param name="picker">The picker that has been configured to
    /// select a file.</param>
    /// <param name="action">The action to be executed on the selected
    /// file.</param>
    template<class TAction> void pick_file_and_continue(
            winrt::Windows::Storage::Pickers::FileSavePicker picker,
            TAction&& action) {
        using namespace winrt::Windows::UI::Core;
        using namespace winrt::Windows::Foundation;
        using namespace winrt::Windows::Storage;
        picker.PickSingleFileAsync().Completed([action](
                const IAsyncOperation<StorageFile> operation,
                const AsyncStatus status) {
            assert(status == AsyncStatus::Completed);
            action(operation.get());
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks a file using the given <paramref name="picker" /> and invokes the
    /// given action with the
    /// <see cref="winrt::Windows::Storage::StorageFile" />.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a single <see cref="winrt::Windows::Storage::StorageFile" />
    /// parameter.</typeparam>
    /// <param name="picker">The file picker used to select the file. The picker
    /// must have been fully configured according to the callers needs.</param>
    /// <param name="action">The action to be invoked for the selected file.
    /// </param>
    template<class TAction> void pick_file_and_continue(
            winrt::Windows::Storage::Pickers::FileOpenPicker picker,
            TAction&& action) {
        using namespace winrt::Windows::UI::Core;
        using namespace winrt::Windows::Foundation;
        using namespace winrt::Windows::Storage;
        picker.PickSingleFileAsync().Completed([action](
                const IAsyncOperation<StorageFile> operation,
                const AsyncStatus status) {
            if (status == AsyncStatus::Completed) {
                action(operation.get());
            } else {
                action(StorageFile(nullptr));
            }
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks one or more files using the given <paramref name="picker" /> and
    /// invokes the given action with a
    /// <see cref="winrt::Windows::Foundation::Collections::IVectorView" /> of
    /// <see cref="winrt::Windows::Storage::StorageFile" />s.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a vector of
    /// <see cref="winrt::Windows::Storage::StorageFile" />s as
    /// parameter.</typeparam>
    /// <param name="picker">The file picker used to select the file. The picker
    /// must have been fully configured according to the callers needs.</param>
    /// <param name="action">The action to be invoked for the selected file.
    /// </param>
    template<class TAction> void pick_files_and_continue(
            winrt::Windows::Storage::Pickers::FileOpenPicker picker,
            TAction&& action) {
        using namespace winrt::Windows::UI::Core;
        using namespace winrt::Windows::Foundation;
        using namespace winrt::Windows::Foundation::Collections;
        using namespace winrt::Windows::Storage;
        picker.PickMultipleFilesAsync().Completed([action](
                const IAsyncOperation<IVectorView<StorageFile>> operation,
                const AsyncStatus status) {
            if (status == AsyncStatus::Completed) {
                action(operation.get());
            } else {
                static auto none = winrt::single_threaded_vector<StorageFile>();
                action(none.GetView());
            }
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks a file using a file picker with the given
    /// <paramref name="filter" /> and invokes the given action with the
    ///  <see cref="winrt::Windows::Storage::StorageFile" />.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a single <see cref="winrt::Windows::Storage::StorageFile" />
    /// parameter.</typeparam>
    /// <param name="filter">The file filter that is applied to the picker that
    /// the function creates for selecting files.</param>
    /// <param name="action">The action to be invoked for the selected file.
    /// </param>
    template<class TAction> void pick_file_and_continue(
            const std::vector<winrt::hstring>& filter,
            TAction&& action) {
        winrt::Windows::Storage::Pickers::FileOpenPicker picker;
        picker.FileTypeFilter().ReplaceAll(filter);
        pick_file_and_continue(picker, std::forward<TAction>(action));
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks one or more files using a file picker with the given
    /// <paramref name="filter" /> and invokes the given action with a
    /// <see cref="winrt::Windows::Foundation::Collections::IVectorView" /> of
    /// <see cref="winrt::Windows::Storage::StorageFile" />s.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a vector of
    /// <see cref="winrt::Windows::Storage::StorageFile" />s as
    /// parameter.</typeparam>
    /// <param name="filter">The file filter that is applied to the picker that
    /// the function creates for selecting files.</param>
    /// <param name="action">The action to be invoked for the selected file.
    /// </param>
    template<class TAction> void pick_files_and_continue(
            const std::vector<winrt::hstring>& filter,
            TAction&& action) {
        winrt::Windows::Storage::Pickers::FileOpenPicker picker;
        picker.FileTypeFilter().ReplaceAll(filter);
        pick_files_and_continue(picker, std::forward<TAction>(action));
    }
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Picks a folder using the given <paramref name="picker" /> and calls the
    /// given action with the selected
    /// <see cref="winrt::Windows::Storage::StorageFolder" />.
    /// </summary>
    /// <typeparam name="TAction">The type of the functor to be invoked, which
    /// must accept a single
    /// <see cref="winrt::Windows::Storage::StorageFolder" /> parameter.
    /// </typeparam>
    /// <param name="picker">The picker that has been configured to
    /// select a folder.</param>
    /// <param name="action">The action to be executed on the selected
    /// folder.</param>
    template<class TAction> void pick_folder_and_continue(
            winrt::Windows::Storage::Pickers::FolderPicker picker,
            TAction&& action) {
        using namespace winrt::Windows::UI::Core;
        using namespace winrt::Windows::Foundation;
        using namespace winrt::Windows::Storage;
        picker.PickSingleFolderAsync().Completed([action](
                const IAsyncOperation<StorageFolder> operation,
                const AsyncStatus status) {
            assert(status == AsyncStatus::Completed);
            action(operation.get());
        });
    }
#endif /* defined(TRROJAN_FOR_UWP) */

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

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// Read a the whole file as text.
    /// </summary>
    /// <param name="file"></param>
    /// <returns></returns>
    std::string TRROJANCORE_API read_text_file(
        const winrt::Windows::Storage::StorageFile file);
#endif /* defined(TRROJAN_FOR_UWP) */

    /// <summary>
    /// Read a whole text file at the location designated by
    /// <see cref="path" />.
    /// </summary>
    /// <param name="path"></param>
    /// <returns></returns>
    inline std::string TRROJANCORE_API read_text_file(const std::string& path) {
        return read_text_file(path.c_str());
    }

#if defined(_WIN32)
    /// <summary>
    /// Write all of the given bytes to the given file.
    /// </summary>
    /// <param name="handle"></param>
    /// <param name="data"></param>
    /// <param name="cnt"></param>
    void TRROJANCORE_API write_all_bytes(HANDLE handle, const void *data,
        const std::size_t cnt);
#endif defined(_WIN32)

    /// <summary>
    /// Specifies the alternative directory separator character if the
    /// platform uses one. Otherwise, this value is equivalent to
    /// <see cref="trrojan::directory_separator_char" />.
    /// </summary>
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

} /* namespace trrojan */

#include "trrojan/io.inl"
