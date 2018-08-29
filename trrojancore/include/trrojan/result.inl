/// <copyright file="result.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::basic_result::check_size
 */
template<class T> void trrojan::basic_result::check_size(const T& results) {
    if (results.size() != this->_result_names.size()) {
        std::stringstream msg;
        msg << "The number of results to be added (" << results.size() << ") "
            "does not match the expected one (" << this->_result_names.size()
            << ")" << std::ends;
        throw std::invalid_argument(msg.str());
    }
}
