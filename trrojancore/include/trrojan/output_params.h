/// <copyright file="output_params.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <memory>
#include <string>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Base class for output parameters, which define what a
    /// <see cref="trrojan::output_base" />-based class should do.
    /// </summary>
    class TRROJANCORE_API basic_output_params {

    public:

        /// <summary>
        /// Create an instance of the specified derived type
        /// <tparamref name="T" />.
        /// </summary>
        /// <tparam name="T"></tparam>
        /// <tparam name="P"></tparam>
        /// <param name="params"></param>
        /// <returns></returns>
        template<class T, class... P>
        static std::shared_ptr<basic_output_params> create(P&&... params) {
            return std::make_shared<T>(std::forward<P>(params)...);
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~basic_output_params(void) = default;

        /// <summary>
        /// Answer the path of the output file.
        /// </summary>
        /// <returns>The output location.</returns>
        const std::string& path(void) const {
            return this->_path;
        }

    protected:

        /// <summary>
        /// Unescapes a character sequence.
        /// </summary>
        static std::string unescape(std::string::const_iterator begin,
            std::string::const_iterator end);

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline basic_output_params(const std::string path) : _path(path) { }

    private:

        basic_output_params(const basic_output_params&) = delete;

        basic_output_params& operator =(const basic_output_params&) = delete;

        std::string _path;

    };

    /// <summary>
    /// Output parameters.
    /// </summary>
    typedef std::shared_ptr<basic_output_params> output_params;
}
