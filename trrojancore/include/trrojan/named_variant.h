/// <copyright file="named_variant.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Valentin Bruder</author>
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

        /// <summary>
        /// Initialises an empty instance.
        /// </summary>
        inline named_variant(void) { }

        /// <summary>
        /// Initialises a new instance with an empty
        /// <see cref="trrojan::variant" />.
        /// </summary>
        /// <param name="name"></param>
        inline explicit named_variant(const std::string& name) : _name(name) { }

        /// <summary>
        /// Initialises a new instance with the given variant as value.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="value"></param>
        inline named_variant(const std::string& name, const variant& value)
            : _name(name), _value(value) { }

        /// <summary>
        /// Initialises a new instance with the given variant as value.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="value"></param>
        inline named_variant(const std::string& name, variant&& value)
            : _name(name), _value(std::move(value)) { }

        /// <summary>
        /// Initialises a new instance with the given value, which must be
        /// convertible to a <see cref="trrojan::variant" />.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="value"></param>
        /// <tparam name="T"></tparam>
        template<class T>
        inline named_variant(const std::string& name, const T value)
            : _name(name), _value(value) { }

        /// <summary>
        /// Initialises a new instance from a pair of string and variant.
        /// </summary>
        /// <remarks>
        /// This constructor can be used to create instances while iterating
        /// an <see cref="std::unordered_map" />.
        /// </remarks>
        /// <param name="name"></param>
        /// <param name="value"></param>
        inline named_variant(const std::pair<std::string, variant>& value)
            : _name(value.first), _value(value.second) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        inline ~named_variant(void) { }

        /// <summary>
        /// Answer the name of the item.
        /// </summary>
        /// <returns></returns>
        inline const std::string& name(void) const {
            return this->_name;
        }

        /// <summary>
        /// Answer the value of the item.
        /// </summary>
        /// <returns></returns>
        inline const variant& value(void) const {
            return this->_value;
        }

        /// <summary>
        /// Implicit conversion to <see cref="trrojan::variant" />.
        /// </summary>
        /// <returns></returns>
        inline operator const variant&(void) const {
            return this->_value;
        }

        /// <summary>
        /// Write the <see cref="trrojan::named_variant" /> to a stream.
        /// </summary>
        /// <param name="lhs">The left-hand side operand (the stream to
        /// write to).</param>
        /// <param name="rhs">The right-hand side operand (the object to
        /// be written).</param>
        /// <returns><paramref name="lhs" />.</returns>
        /// <tparam name="C">The character type used in the stream.</tparam>
        /// <tparam name="T">The traits for <tparamref name="C" />.</tparam>
        template<class C, class T>
        friend inline std::basic_ostream<C, T>& operator <<(
                std::basic_ostream<C, T>& lhs, const named_variant& rhs) {
            lhs << rhs._name << " = \"" << rhs._value << "\"";
            return lhs;
        }

    private:

        std::string _name;
        variant _value;
    };
}
