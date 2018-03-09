/// <copyright file="r_output_params.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::r_output_params::r_output_params
 */
template<class I>
trrojan::r_output_params::r_output_params(const std::string& path,
        I cmdLineBegin, I cmdLineEnd) : basic_output_params(path) {

    {
        auto it = trrojan::find_argument("--line-break", cmdLineBegin,
            cmdLineEnd);
        if (it != cmdLineEnd) {
            this->_line_break = unescape(it->begin(), it->end());
        } else {
            this->_line_break = default_line_break;
        }
    }

    {
        auto it = trrojan::find_argument("--variable-name", cmdLineBegin,
            cmdLineEnd);
        if (it != cmdLineEnd) {
            this->_variable_name = *it;
        } else {
            this->_variable_name = default_variable_name;
        }
    }

    {
        auto it = trrojan::find_argument("--variable-name", cmdLineBegin,
            cmdLineEnd);
        if (it != cmdLineEnd) {
            try {
                this->_expected_rows = parse<size_t>(*it);
            } catch (...) {
                this->_expected_rows = default_expected_rows;
            }
        } else {
            this->_expected_rows = default_expected_rows;
        }
    }
}
