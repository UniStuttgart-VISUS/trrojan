/// <copyright file="console_output.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include "trrojan/console_output_params.h"
#include "trrojan/output.h"


namespace trrojan {

    /// <summary>
    /// Base class for output handlers.
    /// </summary>
    class TRROJANCORE_API console_output : public output_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        console_output(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~console_output(void);

        /// <inheritdoc />
        virtual void close(void);

        /// <inheritdoc />
        virtual void open(const output_params& params);

        /// <inheritdoc />
        virtual output_base& operator <<(const basic_result& result);

    private:

        void print_name(const std::string& str);

        size_t padding;

        std::shared_ptr<console_output_params> params;

    };
}
