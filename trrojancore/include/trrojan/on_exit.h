// <copyright file="on_exit.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/unique_variable.h"


namespace trrojan {
namespace detail {

    /// <summary>
    /// A scope guard object that invokes a user-defined function.
    /// </summary>
    /// <typeparam name="TExitHandler">The type of the lambda expression to
    /// be invoked.Note that the lambda expression passed for this type must
    /// not throw as it might be called from the destructor.</typeparam>
    template<class TExitHandler> class on_exit_guard final {

    public:

        /// <summary>
        /// The type of the lambda expression to be invoked.
        /// </summary>
        typedef TExitHandler exit_handler_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="exitHandler">The lambda to be invoked.</param>
        inline explicit on_exit_guard(exit_handler_type&& exitHandler)
            : _handler(std::forward<exit_handler_type>(exitHandler)),
            _invoked(false) { }

        on_exit_guard(const on_exit_guard&) = delete;

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        inline on_exit_guard(on_exit_guard&& rhs)
            : _handler(std::move(rhs._handler)),
                _invoked(rhs._invoked) {
            rhs._invoked = true;
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        inline ~on_exit_guard(void) {
            this->invoke();
        }

        /// <summary>
        /// If not yet called or cancelled, invoke the exit handler.
        /// </summary>
        void invoke(void) noexcept;

        /// <summary>
        /// Prevent the exit handler from being called.
        /// </summary>
        /// <remarks>
        /// This method marks the handler as being called without actually
        /// calling it. Note that this operation cannot be undone, it will
        /// ultimately prevent the lambda from being called.
        /// </remarks>
        inline void cancel(void) noexcept {
            this->_invoked = true;
        }

        on_exit_guard& operator =(const on_exit_guard&) = delete;

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        /// <returns><c>*this</c></returns>
        on_exit_guard& operator =(on_exit_guard&& rhs);

        /// <summary>
        /// Answer whether the exit handler is still to be invoked.
        /// </summary>
        /// <returns><c>true</c> if the lambda needs to be invoked, <c>false</c>
        /// otherwise.</returns>
        inline operator bool(void) const {
            return !this->_invoked;
        }

    private:

        exit_handler_type _handler;
        bool _invoked;
    };

} /* namespace detail */

    /// <summary>
    /// Create an exit guard that executes <paramref name="exit_handler" /> if
    /// it is destroyed.
    /// </summary>
    /// <typeparam name="TExitHandler">The type of the exit handler function to
    /// be executed.</typeparam>
    /// <param name="exit_handler">The exit handler to be executed.</param>
    /// <returns>A <see cref="on_exit_guard" /> holding
    /// <paramref name="exit_handler" />.</returns>
    template<class TExitHandler>
    detail::on_exit_guard<TExitHandler> make_on_exit_guard(
            TExitHandler&& exit_handler) {
        return detail::on_exit_guard<TExitHandler>(
            std::forward<TExitHandler>(exit_handler));
    }

} /* namespace trrojan */

#include "on_exit.inl"


/// <summary>
/// Declares a <see cref="visus::dataverse::on_exit_guard" /> that executes
/// the <paramref name="exit_handler" /> lambda expression when it is destroyed.
/// </summary>
#define on_exit(exit_handler) auto _TRROJAN_UNIQUE_VARIABLE(_scope_guard) \
= trrojan::make_on_exit_guard(exit_handler)
