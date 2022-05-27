// <copyright file="handle.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include <Windows.h>


namespace trrojan {
namespace d3d12 {

    /// <summary>
    /// RAII wrapper for a <see cref="HANDLE" />.
    /// </summary>
    /// <typeparam name="Invalid">The value that marks and invalid handle.
    /// Typically, this is <c>NULL</c> or <c>INVALID_HANDLE_VALUE</c> depending
    /// on the API from which the handle was derived.
    template<HANDLE Invalid = NULL>
    class handle final {

    public:

        inline handle(void) noexcept : _handle(Invalid) { }

        inline handle(HANDLE&& handle) noexcept : _handle(handle) {
            handle = Invalid;
        }

        handle(const handle&) = delete;

        inline handle(handle&& rhs) noexcept : _handle(rhs._handle) {
            rhs._handle = Invalid;
        }

        ~handle(void);

        handle& operator =(const handle&) = delete;

        handle& operator =(handle&& rhs) noexcept;

        inline operator HANDLE(void) const noexcept {
            return this->_handle;
        }

        inline operator bool(void) const noexcept {
            return (this->_handle != Invalid);
        }

    private:

        HANDLE _handle;
    };

} /* end namespace d3d12 */
} /* end namespace trrojan */

#include "trrojan/d3d12/handle.inl"
