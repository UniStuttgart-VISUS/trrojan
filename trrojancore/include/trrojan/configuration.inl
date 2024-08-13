/// <copyright file="configuration.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


/*
 * trrojan::configuration::get
 */
template<class T>
T trrojan::configuration::get(const std::string& factor) const {
    auto i = this->find(factor);
    if (i == this->_factors.cend()) {
        std::stringstream msg;
        msg << "The configuration does not contain a factor \""
            << factor << "\"." << std::ends;
        throw std::runtime_error(msg.str());
    }
    return i->value().as<T>();
}


/*
 * trrojan::configuration::get
 */
template<class T> T trrojan::configuration::get(const std::string& factor,
        const T fallback) const {
    auto i = this->find(factor);
    return (i != this->_factors.end()) ? i->value().as<T>() : fallback;
}
