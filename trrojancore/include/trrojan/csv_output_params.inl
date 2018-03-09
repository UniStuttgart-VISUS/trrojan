/// <copyright file="csv_output_params.inl" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


/*
 * trrojan::csv_output_params::csv_output_params
 */
template<class I>
trrojan::csv_output_params::csv_output_params(const std::string& path,
        I cmdLineBegin, I cmdLineEnd) : basic_output_params(path) {

    {
        auto it = trrojan::find_argument("--separator", cmdLineBegin,
            cmdLineEnd);
        if (it != cmdLineEnd) {
            this->_separator = unescape(it->begin(), it->end());
        } else {
            this->_separator = default_separator;
        }
    }

    this->_quote_strings = trrojan::contains_switch("--quote-strings",
        cmdLineBegin, cmdLineEnd);

    {
        auto it = trrojan::find_argument("--line-break", cmdLineBegin,
            cmdLineEnd);
        if (it != cmdLineEnd) {
            this->_line_break = unescape(it->begin(), it->end());
        } else {
            this->_line_break = default_line_break;
        }
    }
}
