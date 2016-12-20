/// <copyright file="result.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>

#include "trrojan/export.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// Associates a variant with a user-defined name.
    /// </summary>
    /// <remarks>
    /// Manifestations of <see cref="trrojan::factor" />s and
    /// <see cref="trrojan::result" />s are named variants to unify the output
    /// of a benchmark result.
    /// </remarks>
    class TRROJANCORE_API named_variant {

    public:

        inline explicit named_variant(const std::string& name) : _name(name) { }

        inline named_variant(const std::string& name, const variant& value)
            : _name(name), _value(value) { }

        template<class T>
        inline named_variant(const std::string& name, const T value)
            : _name(name), _value(value) { }

        inline ~named_variant(void) { }

        inline const std::string& name(void) const {
            return this->_name;
        }

        inline const variant& value(void) const {
            return this->_value;
        }

    private:

        std::string _name;

        variant _value;
    };
}
