/// <copyright file="excel_output_params.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/output_params.h"


namespace trrojan {

    /// <summary>
    /// Specialised output parameters for <see cref="trrojan::csv_output" />.
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

        static inline output_params create(const std::string& path,
                const bool is_visible = false) {
            return basic_output_params::create<excel_output_params>(path,
                is_visible);
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="path">The path of the CSV file to be generated.</param>
        inline excel_output_params(const std::string& path,
            const bool is_visible)
            : basic_output_params(path), _is_visible(is_visible) { }

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
