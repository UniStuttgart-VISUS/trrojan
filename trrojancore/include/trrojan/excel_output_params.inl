// <copyright file="excel_output_params.inl" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


/*
 * trrojan::excel_output_params::excel_output_params
 */
template<class I>
trrojan::excel_output_params::excel_output_params(const std::string& path,
        I cmdLineBegin, I cmdLineEnd) : basic_output_params(path) {
    this->_is_visible = trrojan::contains_switch("--visible", cmdLineBegin,
        cmdLineEnd);
}
