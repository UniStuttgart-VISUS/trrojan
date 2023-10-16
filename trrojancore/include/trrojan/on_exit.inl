// <copyright file="on_exit.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::detail::on_exit_guard<TExitHandler>::invoke
 */
template<class TExitHandler>
void trrojan::detail::on_exit_guard<TExitHandler>::invoke(void) noexcept {
    if (*this) {
        this->_handler();
        this->_invoked = true;
    }
}


/*
 * trrojan::detail::on_exit_guard<TExitHandler>::operator =
 */
template<class TExitHandler>
trrojan::detail::on_exit_guard<TExitHandler>&
trrojan::detail::on_exit_guard<TExitHandler>::operator =(
        on_exit_guard&& rhs) {
    if (this != std::addressof(rhs)) {
        this->_handler = std::move(rhs._handler);
        this->_invoked = rhs._invoked;
        rhs._invoked = true;
    }

    return *this;
}
