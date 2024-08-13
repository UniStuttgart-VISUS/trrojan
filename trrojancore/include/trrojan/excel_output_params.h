// <copyright file="excel_output_params.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/cmd_line.h"
#include "trrojan/output_params.h"


namespace trrojan {

    /// <summary>
    /// Specialised output parameters for <see cref="trrojan::excel_output" />.
    /// </summary>
    class TRROJANCORE_API excel_output_params : public basic_output_params {

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
        ///// Creates a new instance.
        ///// </summary>
        //static inline output_params create(const std::string& path,
        //        const bool is_visible = false) {
        //    return basic_output_params::create<excel_output_params>(path,
        //        is_visible);
        //}

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path of the Excel file to be generated.
        /// </param>
        /// <param name="is_visible">Determines whether a visible Excel window
        /// is opened.</param>
        inline excel_output_params(const std::string& path,
                const bool is_visible)
            : basic_output_params(path), _is_visible(is_visible) { }

        /// <summary>
        /// Initialises a new instance from a command line.
        /// </summary>
        /// <param name="path">The path of the Excel file to be generated.
        /// </param>
        /// <param name="cmdLineBegin">Begin of the command line arguments.
        /// </param>
        /// <param name="cmdLineEnd">End of the command line arguments.</param>
        template<class I> excel_output_params(const std::string& path,
            I cmdLineBegin, I cmdLineEnd);

        /// <summary>
        /// Initialises a new instance with the given basic parameters.
        /// </summary>
        inline explicit excel_output_params(const basic_output_params& params)
            : basic_output_params(params.path()), _is_visible(false) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~excel_output_params(void);

        inline const bool is_visible(void) const {
            return this->_is_visible;
        }

    private:

        bool _is_visible;

    };

}

#include "trrojan/excel_output_params.inl"
