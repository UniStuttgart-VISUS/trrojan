/// <copyright file="excel_output_params.inl" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


/*
 * trrojan::excel_output_params::excel_output_params
 */
template<class I>
trrojan::excel_output_params::excel_output_params(const std::string& path,
        I cmdLineBegin, I cmdLineEnd) : basic_output_params(path) {
    this->_is_visible = trrojan::contains_switch("--visible", cmdLineBegin,
        cmdLineEnd);
}
