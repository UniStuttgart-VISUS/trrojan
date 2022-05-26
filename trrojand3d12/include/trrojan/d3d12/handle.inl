// <copyright file="handle.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::d3d12::handle<Invalid>::~handle
 */
template<HANDLE Invalid>
trrojan::d3d12::handle<Invalid>::~handle(void) {
    if (*this) {
        ::CloseHandle(this->_handle);
    }
}


/*
 * trrojan::d3d12::handle<Invalid>::operator =
 */
template<HANDLE Invalid>
trrojan::d3d12::handle<Invalid>&
trrojan::d3d12::handle<Invalid>::operator =(handle&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        this->_handle = rhs._handle;
        rhs._handle = Invalid;
    }

    return *this;
}
