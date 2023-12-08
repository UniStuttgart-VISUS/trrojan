// <copyright file="temp_file.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <string>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// RAII container for a temporary file that will self-destruct if it is
    /// destroyed or overridden.
    /// </summary>
    class TRROJANCORE_API temp_file final {

    public:

        /// <summary>
        /// Create a new temporary file name using the given prefix.
        /// </summary>
        /// <param name="prefix">The prefix of the file name. If this is
        /// <c>nullptr</c>, &quot;trrojan&quot; will be used.</param>
        /// <returns></returns>
        static temp_file create(const char *prefix = nullptr);

        /// <summary>
        /// Create a new temporary file name using the given prefix in the given
        /// folder.
        /// </summary>
        /// <param name="folder">The path to the temporary folder. If this is
        /// <c>nullptr</c>, <see cref="get_temp_path" /> will be used.</param>
        /// <param name="prefix">The prefix of the file name. If this is
        /// <c>nullptr</c>, &quot;trrojan&quot; will be used.</param>
        /// <returns></returns>
        static temp_file create(const char *folder, const char *prefix);

        /// <summary>
        /// Make the existing path a temporary file that is automatically being
        /// deleted.
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        static temp_file from_path(const std::string& path) {
            return temp_file(path);
        }

        /// <summary>
        /// Initialises an invalid temporary file.
        /// </summary>
        inline temp_file(void) noexcept { }

        temp_file(const temp_file& rhs) = delete;

        temp_file(temp_file&& rhs) noexcept;

        ~temp_file(void) noexcept;

        /// <summary>
        /// Answer the path of the temporary file.
        /// </summary>
        /// <returns></returns>
        inline const std::string& get(void) const noexcept {
            return this->_path;
        }

        /// <summary>
        /// Deletes the file if it is valid.
        /// </summary>
        void release(void) noexcept;

        temp_file& operator =(const temp_file& rhs) = delete;

        temp_file& operator =(temp_file&& rhs) noexcept;

        /// <summary>
        /// Answer whether the file is valid.
        /// </summary>
        inline operator bool(void) const noexcept {
            return !this->_path.empty();
        }

        /// <summary>
        /// Answer the path of the temporary file.
        /// </summary>
        inline operator const std::string&(void) const noexcept {
            return this->_path;
        }

        /// <summary>
        /// Answer the path of the temporary file.
        /// </summary>
        operator std::wstring(void) const;

    private:

        explicit temp_file(const std::string& path);

        std::string _path;
    };

} /* namespace trrojan */
