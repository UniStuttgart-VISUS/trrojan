/// <copyright file="csv_output_params.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/output_params.h"


namespace trrojan {

    /// <summary>
    /// Specialised output parameters for <see cref="trrojan::csv_output" />.
    /// </summary>
    class TRROJANCORE_API csv_output_params : public basic_output_params {

    public:

        /// <summary>
        /// The default line break string.
        /// </summary>
        static const std::string default_line_break;

        /// <summary>
        /// The default column separator string.
        /// </summary>
        static const std::string default_separator;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path of the CSV file to be generated.</param>
        inline csv_output_params(const std::string& path,
            const std::string& separator = default_separator,
            const bool quote_strings = true,
            const std::string& line_break = default_line_break)
            : basic_output_params(path), _line_break(line_break),
            _quote_strings(quote_strings), _separator(separator) { }

        inline explicit csv_output_params(const basic_output_params& params)
            : basic_output_params(params.path()),
            _line_break(default_line_break), _quote_strings(true),
            _separator(default_separator) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~csv_output_params(void);

        inline const std::string& line_break(void) const {
            return this->_line_break;
        }

        inline bool quote_string(void) const {
            return this->_quote_strings;
        }

        inline std::string separator(void) const {
            return this->_separator;
        }

    private:

        std::string _line_break;

        bool _quote_strings;

        std::string _separator;

    };

}
