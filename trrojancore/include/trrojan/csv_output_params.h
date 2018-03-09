/// <copyright file="csv_output_params.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 - 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/cmd_line.h"
#include "trrojan/output_params.h"
#include "trrojan/text.h"


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

        ///// <summary>
        ///// Create a new instance.
        ///// <summary>
        ///// <param name="separator"></param>
        ///// <param name="quote_strings"></param>
        ///// <param name="line_break"></param>
        ///// <returns></returns>
        //static inline output_params create(const std::string& path,
        //        const std::string& separator = default_separator,
        //        const bool quote_strings = true,
        //        const std::string& line_break = default_line_break) {
        //    return basic_output_params::create<csv_output_params>(path,
        //        separator, quote_strings, line_break);
        //}

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path of the CSV file to be generated.</param>
        /// <param name="separator"></param>
        /// <param name="quote_strings"></param>
        /// <param name="line_break"></param>
        inline csv_output_params(const std::string& path,
            const std::string& separator, const bool quote_strings,
            const std::string& line_break)
            : basic_output_params(path), _line_break(line_break),
            _quote_strings(quote_strings), _separator(separator) { }

        /// <summary>
        /// Initialises a new instance from a command line.
        /// </summary>
        /// <param name="path">The path of the CSV file to be generated.</param>
        /// <param name="cmdLineBegin">Begin of the command line arguments.
        /// </param>
        /// <param name="cmdLineEnd">End of the command line arguments.</param>
        template<class I> csv_output_params(const std::string& path,
            I cmdLineBegin, I cmdLineEnd);

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

        static std::string unescape(std::string::const_iterator begin,
            std::string::const_iterator end);

        std::string _line_break;

        bool _quote_strings;

        std::string _separator;

    };

}

#include "trrojan/csv_output_params.inl"
