// <copyright file="console_output_params.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/output_params.h"


namespace trrojan {

    /// <summary>
    /// Specialised output parameters for
    /// <see cref="trrojan::console_output" />.
    /// </summary>
    class TRROJANCORE_API console_output_params : public basic_output_params {

    public:

        /// <summary>
        /// Creates a new instance.
        /// </summary>
        static inline output_params create(void) {
            return basic_output_params::create<console_output_params>();
        }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline console_output_params(void) : basic_output_params("") { }

        /// <summary>
        /// Initialises a new instance with the given basic parameters.
        /// </summary>
        inline explicit console_output_params(const basic_output_params& params)
            : basic_output_params(params.path()) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~console_output_params(void) = default;

    };

}
