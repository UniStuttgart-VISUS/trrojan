// <copyright file="com_error_category.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#if defined(_WIN32)
#include <system_error>

#include <Windows.h>

#include "trrojan/export.h"


namespace trrojan {
namespace detail {

    /// <summary>
    /// An error category for <c>HRESULT</c>s from COM.
    /// </summary>
    class TRROJANCORE_API com_error_category final
            : public std::error_category {

    public:

        using std::error_category::error_category;

        /// <summary>
        /// Converts the error code into a portable description.
        /// </summary>
        /// <param name="error">The error code to be converted.</param>
        /// <returns>The portable description of the COM error.</returns>
        std::error_condition default_error_condition(
            _In_ int error) const noexcept override;

        /// <summary>
        /// Convert the given error code into a string.
        /// </summary>
        /// <param name="error">The error code to get the message for.</param>
        /// <returns>The error message associated with the error code.
        /// </returns>
        std::string message(_In_ int error) const override;

        /// <summary>
        /// Answer the name of the error category.
        /// </summary>
        /// <returns>The name of the category.</returns>
        inline _Ret_z_ const char *name(void) const noexcept override {
            return "Component Object Model";
        }
    };

}  /* namespace detail */


    /// <summary>
    /// Answer the one and only <see cref="com_error_category" />.
    /// </summary>
    /// <returns>The only instance of <see cref="com_error_category" />.
    /// </returns>
    _Ret_valid_ extern TRROJANCORE_API const std::error_category& com_category(
        void) noexcept;

} /* namespace trrojan */


namespace std {

    /// <summary>
    /// Tell STL that <c>HRESULT</c>s are error codes.
    /// </summary>
    template<> struct is_error_code_enum<HRESULT> : public true_type { };

    /// <summary>
    /// Allow STL to convert an COM error code into a generic error code.
    /// </summary>
    /// <param name="e">The OpenSSL error to be converted.</param>
    /// <returns>The generic error code.</returns>
    inline std::error_code make_error_code( _In_ const HRESULT e) noexcept {
        return std::error_code(static_cast<int>(e), trrojan::com_category());
    }
} /* namespace std */

#endif /* defined(_WIN32) */
