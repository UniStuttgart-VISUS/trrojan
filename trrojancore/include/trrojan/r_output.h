/// <copyright file="r_output.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <fstream>

#include "trrojan/r_output_params.h"
#include "trrojan/output.h"


namespace trrojan {

    /// <summary>
    /// Output handler writing the results to an R script.
    /// </summary>
    class TRROJANCORE_API r_output : public output_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        r_output(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~r_output(void);

        /// <inheritdoc />
        virtual void close(void);

        /// <inheritdoc />
        virtual void open(const output_params& params);

        /// <inheritdoc />
        virtual output_base& operator <<(const basic_result& result);

    private:

        typedef r_output_params params_type;

        void print(const std::string& str);

        void print(const variant& v);

        inline void print(const named_variant& v) {
            this->print(v.value());
        }

        void print_factor(const named_variant& v);

        std::stringstream conversion;
        std::ofstream file;
        std::uint64_t line;
        std::shared_ptr<params_type> params;
    };
}
