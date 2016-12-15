/// <copyright file="factor.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cmath>
#include <memory>

#include "factor_base.h"


namespace trrojan {
namespace detail {

    /// <summary>
    /// Base class for different implementations of a
    /// <see cref="trrojan::factor" />.
    /// </summary>
    /// <tparam name="T">A numeric type that can be interpolated.</tparam>
    template<class T> class factor_range : public factor_base {

    public:

        /// <summary>
        /// The type of value returned by the factor.
        /// </summary>
        typedef T value_type;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        inline factor_range(void) : begin(0), cnt_steps(0), step_size(0) { }

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        /// <param name="name"></param>
        /// <param name="begin"></param>
        /// <param name="step_size"></param>
        /// <param name="cnt_steps"></param>
        inline factor_range(const std::string& name, const value_type begin,
            const value_type step_size, const size_t cnt_steps)
            : factor_base(name), begin(begin), cnt_steps(cnt_steps),
            step_size(step_size) { }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~factor_range(void);

        /// <inheritdoc />
        virtual std::unique_ptr<factor_base> clone(void) const;

        /// <inheritdoc />
        virtual size_t size(void) const;

        /// <inheritdoc />
        virtual const variant& operator [](const size_t i) const;

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>true</c> if this object and <paramref name="rhs" />
        /// are equal, <c>false</c> otherwise.</returns>
        inline bool operator ==(const factor_range& rhs) const {
            return (factor_base::operator ==(rhs)
                && (this->begin == rhs.begin)
                && (this->cnt_steps == rhs.cnt_steps)
                && (this->step_size == rhs.step_size));
        }

        /// <summary>
        /// Test for inequality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>false</c> if this object and <paramref name="rhs" />
        /// are equal, <c>true</c> otherwise.</returns>
        inline bool operator !=(const factor_range& rhs) const {
            return !(*this == rhs);
        }

    private:

        /// <summary>
        /// The first value.
        /// </summary>
        value_type begin;

        /// <summary>
        /// The number of manifestations to generate.
        /// </summary>
        size_t cnt_steps;

        /// <summary>
        /// The step size between the manifestations.
        /// </summary>
        value_type step_size;
    };

    /// <summary>
    /// Factory method for creating a new
    /// <see cref="trrojan::detail::factor_range" /> from a start value and an
    /// end value and a number of steps.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="step_size"></param>
    /// <param name="cnt_steps"></param>
    /// <returns></returns>
    template<class T> std::unique_ptr<factor_range<T>> make_factor_from_range(
            const std::string& name, const T begin, const T end,
            const size_t cnt_steps) {
        auto range = static_cast<double>(std::abs(end - begin) + 1);
        auto retval = std::unique_ptr<factor_range<T>>(new factor_range<T>(name,
            begin, static_cast<T>(range / cnt_steps), cnt_steps));
        return std::move(retval);
    }

    /// <summary>
    /// Factory method for creating a new
    /// <see cref="trrojan::detail::factor_range" /> from a start value, a step
    /// size and a number of steps.
    /// </summary>
    /// <param name="name"></param>
    /// <param name="begin"></param>
    /// <param name="step_size"></param>
    /// <param name="cnt_steps"></param>
    /// <returns></returns>
    template<class T> std::unique_ptr<factor_range<T>> make_factor_from_steps(
            const std::string& name, const T begin, const T step_size,
            const size_t cnt_steps) {
        auto retval = std::unique_ptr<factor_range<T>>(new factor_range<T>(name,
            begin, step_size, cnt_steps));
        return std::move(retval);
    }
}
}

#include "trrojan/factor_range.inl"
