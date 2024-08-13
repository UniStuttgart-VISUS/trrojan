// <copyright file="lazy.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::lazy<TValue>::get
 */
template<class TValue>
typename trrojan::lazy<TValue>::value_type& trrojan::lazy<TValue>::get(void) {
    if (!this->_valid) {
        this->_value = this->_generator();
        this->_valid = true;
    }

    return this->_value;
}


/*
 * trrojan::lazy<TValue>::get
 */
template<class TValue>
const typename trrojan::lazy<TValue>::value_type& trrojan::lazy<TValue>::get(
        void) const {
    if (!this->_valid) {
        throw std::logic_error("The first access to a lazy variable must not "
            "happen on a constant instance.");
    }

    return this->_value;
}
