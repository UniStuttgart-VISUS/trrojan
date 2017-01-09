/// <copyright file="factor.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <algorithm>
#include <memory>

#include "trrojan/export.h"
#include "trrojan/factor_base.h"
#include "trrojan/factor_enum.h"
#include "trrojan/factor_range.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// This class represents a variable factor which impacts performance.
    /// </summary>
    /// <remarks>
    /// <para>We use the facade pattern in order to allow for the factor living
    /// on the stack and hiding different kinds of factors which implement the
    /// same functionality differently.</para>
    /// </remarks>
    class TRROJANCORE_API factor {

    public:

        /// <summary>
        /// Creates a new factor with a set of discrete manifestations.
        /// </summary>
        /// <parama name="name">The name which uniquely identifies the factor.
        /// </param>
        /// <parama name="manifestations">The manifestations of the factor.
        /// </param>
        /// <returns>A new instance with the specified manifestations.</returns>
        /// <tparam name="T">The value type of the factor.</tparam>
        template<class T> static inline factor from_manifestations(
                const std::string& name, const std::vector<T>& manifestations) {
            std::vector<trrojan::variant> m;
            std::transform(manifestations.cbegin(), manifestations.cend(),
                std::back_inserter(m), [](const T& n) { return trrojan::variant(n); });
            return factor(new detail::factor_enum(name, std::move(m)));
        }

        /// <summary>
        /// Creates a new factor from a set of discrete manifestations.
        /// </summary>
        /// <parama name="name">The name which uniquely identifies the factor.
        /// </param>
        /// <parama name="begin">The begin of the range of manifestations.
        /// </param>
        /// <parama name="end">The end of the range of manifestations.</param>
        /// <returns>A new instance with the specified manifestations.</returns>
        /// <tparam name="T">The value type of the factor.</tparam>
        template<class I> static inline factor from_manifestations(
                const std::string& name, I begin, I end) {
            return factor(new detail::factor_enum(name, begin, end));
        }

        /// <summary>
        /// Creates a new factor with a set of discrete manifestations.
        /// </summary>
        /// <parama name="name">The name which uniquely identifies the factor.
        /// </param>
        /// <parama name="manifestations">The manifestations of the factor.
        /// </param>
        /// <returns>A new instance with the specified manifestations.</returns>
        /// <tparam name="T">The value type of the factor.</tparam>
        template<class T> static inline factor from_manifestations(
                const std::string& name,
                const std::initializer_list<T>& manifestations) {
            std::vector<T> m = manifestations;
            return factor::from_manifestations(name, m);
        }

        /// <summary>
        /// Creates a new factor with a single manifestation.
        /// </summary>
        /// <parama name="name">The name which uniquely identifies the factor.
        /// </param>
        /// <parama name="manifestation">The manifestation of the factor.
        /// </param>
        /// <returns>A new instance with the specified manifestation.</returns>
        /// <tparam name="T">The value type of the factor.</tparam>
        template<class T> static inline factor from_manifestations(
            const std::string& name, const T& manifestation) {
            std::vector<T> m(1, manifestation);
            return factor::from_manifestations(name, m);
        }

        template<class T>
        static inline factor from_range(const std::string& name, const T begin,
            const T end, const size_t cnt_steps) {
            return factor(detail::make_factor_from_range(name, begin,
                end, cnt_steps));
        }

        template<class T>
        static inline factor from_steps(const std::string& name, const T begin,
                const T step_size, const size_t cnt_steps) {
            return factor(detail::make_factor_from_steps(name, begin,
                step_size, cnt_steps));
        }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be cloned.</param>
        factor(const factor& rhs);

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        inline factor(factor&& rhs) : impl(std::move(rhs.impl)) { }

        /// <summary>
        /// Answers the name of the factor.
        /// </summary>
        /// <returns>The name of the factor</returns>
        inline const std::string& name(void) const {
            static const std::string EMPTY;
            return (this->impl != nullptr) ? this->impl->name() : EMPTY;
        }

        /// <summary>
        /// Answer the number of different manifestations the factor has.
        /// </summary>
        /// <returns>The number of manifestations.</returns>
        inline size_t size(void) const {
            return (this->impl != nullptr) ? this->impl->size() : 0;
        }

        /// <summary>
        /// Answer a specific manifestation.
        /// </summary>
        /// <remarks>
        /// The method returns a deep copy to allow implementations generating
        /// factors on-the-fly. If we required a reference to be returned, this
        /// would result in returning a reference to a temporary.
        /// </remarks>
        /// <param name="i"></param>
        /// <returns>The <paramref name="i" />th manifestation.</returns>
        /// <exception cref="std::range_error"></exception>
        variant operator [](const size_t i) const;

        /// <summary>
        /// Assignment.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        factor& operator =(const factor& rhs);

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        factor& operator =(factor&& rhs);

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>true</c> if this object and <paramref name="rhs" />
        /// are equal, <c>false</c> otherwise.</returns>
        bool operator ==(const factor& rhs) const;

        /// <summary>
        /// Test for inequality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>false</c> if this object and <paramref name="rhs" />
        /// are equal, <c>true</c> otherwise.</returns>
        inline bool operator !=(const factor& rhs) const {
            return !(*this == rhs);
        }

    private:

        inline factor(detail::factor_base *impl) : impl(impl) { }

        inline factor(std::unique_ptr<detail::factor_base> impl)
            : impl(std::move(impl)) { }

        /// <summary>
        /// Pointer to the actual implementation.
        /// </summary>
        std::unique_ptr<detail::factor_base> impl;
    };
}
