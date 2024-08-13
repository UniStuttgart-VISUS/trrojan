/// <copyright file="r_output_params.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/cmd_line.h"
#include "trrojan/output_params.h"
#include "trrojan/text.h"


namespace trrojan {

    /// <summary>
    /// Specialised output parameters for <see cref="trrojan::r_output" />.
    /// </summary>
    class TRROJANCORE_API r_output_params : public basic_output_params {

    public:

        /// <summary>
        /// The default value for the expected number of data rows.
        /// </summary>
        static const size_t default_expected_rows = 1;

        /// <summary>
        /// The default line break string.
        /// </summary>
        static const std::string default_line_break;

        /// <summary>
        /// The default name of the variable to be filled with the data.
        /// </summary>
        static const std::string default_variable_name;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path of the R file to be generated.</param>
        /// <param name="line_break"></param>
        inline r_output_params(const std::string& path,
                const std::string& line_break, const std::string& variable_name,
                const size_t expected_rows)
            : basic_output_params(path), _expected_rows(expected_rows),
                _line_break(line_break), _variable_name(variable_name) { }

        /// <summary>
        /// Initialises a new instance from a command line.
        /// </summary>
        /// <param name="path">The path of the CSV file to be generated.</param>
        /// <param name="cmdLineBegin">Begin of the command line arguments.
        /// </param>
        /// <param name="cmdLineEnd">End of the command line arguments.</param>
        template<class I> r_output_params(const std::string& path,
            I cmdLineBegin, I cmdLineEnd);

        inline explicit r_output_params(const basic_output_params& params)
                : basic_output_params(params.path()),
            _expected_rows(default_expected_rows),
            _line_break(default_line_break),
            _variable_name(default_variable_name) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~r_output_params(void) = default;

        inline size_t expected_rows(void) const {
            return this->_expected_rows;
        }

        inline const std::string& line_break(void) const {
            return this->_line_break;
        }

        inline const std::string& variable_name(void) const {
            return this->_variable_name;
        }

    private:

        size_t _expected_rows;

        std::string _line_break;

        std::string _variable_name;

    };

}

#include "trrojan/r_output_params.inl"
