// <copyright file="com_error_category.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 - 2024 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/com_error_category.h"

#if defined(_WIN32)
#include <winrt/base.h>

#include "trrojan/text.h"


/*
 * trrojan::detail::com_error_category::default_error_condition
 */
std::error_condition
trrojan::detail::com_error_category::default_error_condition(
        _In_ int error) const noexcept {
    auto code = HRESULT_CODE(error);

    if (code || (error == ERROR_SUCCESS)) {
        // 'hr' is a default Windows code mapped to HRESULT.
        return std::system_category().default_error_condition(error);
    } else {
        // This is an actual COM error.
        return std::error_condition(error, com_category());
    }
}


/*
 * trrojan::detail::com_error_category::message
 */
std::string trrojan::detail::com_error_category::message(_In_ int error) const {
    winrt::hresult_error e(error);
    return winrt::to_string(e.message());
}


/*
 * trrojan::com_category
 */
_Ret_valid_ const std::error_category& trrojan::com_category(void) noexcept {
    static const detail::com_error_category retval;
    return retval;
}
#endif /* defined(_WIN32) */
