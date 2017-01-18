/// <copyright file="configuration.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <iterator>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "trrojan/export.h"
#include "trrojan/named_variant.h"


namespace trrojan {

    /// <summary>
    /// A configuration, which is defined as a set of manifestations of
    /// (named) factors.
    /// </summary>
    class TRROJANCORE_API configuration {

    public:

        typedef std::vector<named_variant> container_type;
        typedef container_type::const_iterator iterator_type;
        typedef container_type::value_type value_type;

        /// <summary>
        /// Create a copy of <paramref name="cfg" /> which contains all
        /// system factors.
        /// </summary>
        static configuration with_system_factors(configuration cfg) {
            cfg.add_system_factors();
            return std::move(cfg);
        }

        /// <summary>
        /// Adds a new factor to the configuation.
        /// </summary>
        inline void add(const named_variant& factor) {
            this->check_duplicate(factor.name());
            this->_factors.push_back(factor);
        }

        /// <summary>
        /// Adds a new factor to the configuation.
        /// </summary>
        inline void add(named_variant&& factor) {
            this->check_duplicate(factor.name());
            this->_factors.push_back(std::move(factor));
        }

        /// <summary>
        /// Adds a new factor to the configuation.
        /// </summary>
        inline void add(const std::string& name, const trrojan::variant& value) {
            this->check_duplicate(name);
            this->_factors.emplace_back(name, value);
        }

        /// <summary>
        /// Adds a new factor to the configuation.
        /// </summary>
        inline void add(const std::string& name, trrojan::variant&& value) {
            this->check_duplicate(name);
            this->_factors.emplace_back(name, std::move(value));
        }

        /// <summary>
        /// Adds a new factor to the configuation.
        /// </summary>
        inline void add(std::string&& name, trrojan::variant&& value) {
            this->check_duplicate(name);
            this->_factors.emplace_back(std::move(name), std::move(value));
        }

        /// <summary>
        /// Adds all <see cref="trrojan::system_factor" />s to the configuration.
        /// </summary>
        void add_system_factors(void);

        /// <summary>
        /// Gets an iterator for the begin of the factors.
        /// </summary>
        inline iterator_type begin(void) const {
            return this->_factors.cbegin();
        }

        /// <summary>
        /// Ensure that this configuration and <paramref name="other" /> contain
        /// the same factors or throw an exception.
        /// </summary>
        /// <param name="other">The configuration to be compared.</param>
        /// <exception cref="std::runtime_error">If the configurations do not
        /// hold the same factors.</exception>
        void check_consistency(const configuration& other) const;

        /// <summary>
        /// Removes all factors from the configuration.
        /// </summary>
        inline void clear(void) {
            this->_factors.clear();
        }

        /// <summary>
        /// Answer whether the configuration contains a factor with the
        /// specified name.
        /// </summary>
        inline bool contains(const std::string& factor) const {
            return (this->find(factor) != this->_factors.cend());
        }

        /// <summary>
        /// Gets an iterator for the end of the factors.
        /// </summary>
        inline iterator_type end(void) const {
            return this->_factors.cend();
        }

        /// <summary>
        /// Find the factor with the specified name.
        /// </summary>
        iterator_type find(const std::string& factor) const;

        /// <summary>
        /// Get the value of the given factor as type <tparamref name="T" /> or
        /// raise an exception if the factor does not exist or is incompatible.
        /// </summary>
        /// <param name="factor">The name of the factor to retrieve.</param>
        /// <returns>The value of the requested factor.</returns>
        /// <tparam name="T">The type of the factor to retrieve.</tparam>
        /// <exception cref="std::bad_cast">If the factor exists, but has an
        /// incompatible type.</exception>
        /// <exception cref="std::runtime_error">If the factor does not exist.
        /// </exception>
        template<class T> T get(const std::string& factor) const;

        /// <summary>
        /// Get the value of the given factor as type <tparamref name="T" /> or
        /// return the <paramref name="fallback" /> value if the factor does not
        /// exist.
        /// </summary>
        /// <param name="factor">The name of the factor to retrieve.</param>
        /// <param name="fallback">The value to use of the factor does not exist.
        /// </param>
        /// <returns>The value of the requested factor.</returns>
        /// <tparam name="T">The type of the factor to retrieve.</tparam>
        /// <exception cref="std::bad_cast">If the factor exists, but has an
        /// incompatible type.</exception>
        template<class T>
        T get(const std::string& factor, const T fallback) const;

        /// <summary>
        /// Makes sure that the configuration has reserved space for the given
        /// number of factors.
        /// </summary>
        inline void reserve(const size_t size) {
            this->_factors.reserve(size);
        }

        /// <summary>
        /// Answer the <paramref name="i" />th factor.
        /// </summary>
        inline const value_type& operator [](const size_t i) const {
            return this->_factors[i];
        }

        /// <summary>
        /// Answer the number of factors.
        /// </summary>
        inline const std::size_t size(void) const {
            return this->_factors.size();
        }

        /// <summary>
        /// Write a <see cref="trrojan::configuration" /> to a stream.
        /// </summary>
        /// <param name="lhs">The left-hand side operand (the stream to
        /// write to).</param>
        /// <param name="rhs">The right-hand side operand (the object to
        /// be written).</param>
        /// <returns><paramref name="lhs" />.</returns>
        /// <tparam name="C">The character type used in the stream.</tparam>
        /// <tparam name="T">The traits for <tparamref name="C" />.</tparam>
        template<class C, class T>
        inline friend std::basic_ostream<C, T>& operator <<(
                std::basic_ostream<C, T>& lhs, const configuration& rhs) {
            bool isFirst = true;

            for (auto& f : rhs._factors) {
                if (isFirst) {
                    isFirst = false;
                } else {
                    lhs << ", ";
                }

                lhs << f;
            }

            return lhs;
        }

    private:

        void check_duplicate(const std::string& name);

        container_type _factors;

    };

    /// <summary>
    /// Answer the configuration as name-value pairs in a string.
    /// </summary>
    std::string to_string(const configuration& c);
}

#include "trrojan/configuration.inl"
