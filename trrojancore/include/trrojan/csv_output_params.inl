/// <copyright file="csv_output_params.inl" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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

    this->_quote_strings = !trrojan::contains_switch("--do-not-quote-strings",
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
