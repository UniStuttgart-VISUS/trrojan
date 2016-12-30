/// <copyright file="output.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>

#include "trrojan/configuration.h"
#include "trrojan/export.h"
#include "trrojan/result.h"


namespace trrojan {

    /// <summary>
    /// Base class for output handlers.
    /// </summary>
    class TRROJANCORE_API output_base {

    public:

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~output_base(void);

        /// <summary>
        /// Stores the given benchmark <see cref="trrojan::result" /> in the
        /// output.
        /// </summary>
        /// <param name="result"></param>
        /// <returns><c>*this</c></returns>
        virtual output_base& operator <<(const basic_result& result) = 0;

        /// <summary>
        /// Stores the given benchmark <see cref="trrojan::result" /> in the
        /// output.
        /// </summary>
        /// <remarks>
        /// It is safe to pass <c>nullptr</c>, in which case nothing will
        /// happen.
        /// </remarks>
        /// <param name="result"></param>
        /// <returns><c>*this</c></returns>
        output_base& operator <<(const result result);
        
        // TODO: define the interface.

    };

    /// <summary>
    /// An output handler.
    /// </summary>
    typedef std::shared_ptr<output_base> output;
}
