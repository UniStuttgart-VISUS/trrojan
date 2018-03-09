/// <copyright file="excel_output_params.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::excel_output_params::excel_output_params
 */
template<class I>
trrojan::excel_output_params::excel_output_params(const std::string& path,
        I cmdLineBegin, I cmdLineEnd) : basic_output_params(path) {
    this->_is_visible = trrojan::contains_switch("--visible", cmdLineBegin,
        cmdLineEnd);
}
