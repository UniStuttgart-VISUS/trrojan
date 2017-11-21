/// <copyright file="result.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
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
