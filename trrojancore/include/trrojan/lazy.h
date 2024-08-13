// <copyright file="lazy.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>


namespace trrojan {

    /// <summary>
    /// A lazy generator and store for objects of type
    /// <typeparamref name="TValue" />.
    /// </summary>
    /// <typeparam name="TValue">The type of the variable to be lazily created.
    /// </typeparam>
    template<class TValue> class lazy {

    public:

        /// <summary>
        /// The type of the variable to be lazily created.
        /// </summary>
        typedef TValue value_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <typeparam name="TGenerator">The type of a functor that takes no
        /// argument and creates an instance of <see cref="value_type "/>.
        /// </typeparam>
        /// <param name="generator">The generator that creates the value on
        /// first access.</param>
        template<class TGenerator>
        explicit inline lazy(const TGenerator& generator)
            : _generator(generator), _valid(false) { }

        /// <summary>
        /// Gets the lazily created variable.
        /// </summary>
        /// <returns>The value that is stored in this variable, after
        /// potentially creating it if it was invalid before.</returns>
        value_type& get(void);

        /// <summary>
        /// Gets the value, but only if it has been created before.
        /// </summary>
        /// <returns>The value stored in the variable.</returns>
        /// <exception cref="std::logic_error">If the variable has not been
        /// lazily initialised before. The first access to the variable must
        /// be on a non-constant instance.</exception>
        const value_type& get(void) const;

        /// <summary>
        /// Resets the instance to the given (invalid) value and enables the
        /// generator next time the variable is accessed.
        /// </summary>
        /// <param name="invalid_value">A value that is considered invalid. You
        /// can also use a valid value, but it will be overwritten next time the
        /// lazy variable is accessed.</param>
        void reset(const value_type invalid_value) {
            this->_value = invalid_value;
            this->_valid = false;
        }

        /// <summary>
        /// Resets the instance to create a new value next time the variable is
        /// accessed, but does not erase the existing value.
        /// </summary>
        /// <remarks>
        /// If your code relies on previous values being released, use the
        /// overload tha allows you to set an invalid value.
        /// </remarks>
        void reset(void) {
            this->_valid = false;
        }

        /// <summary>
        /// Gets the lazily created variable.
        /// </summary>
        /// <returns>The value that is stored in this variable, after
        /// potentially creating it if it was invalid before.</returns>
        inline operator value_type&(void) {
            return this->get();
        }

    private:

        std::function<value_type(void)> _generator;
        value_type _value;
        bool _valid;
    };

} /* end namespace trrojan */

#include "trrojan/lazy.inl"
