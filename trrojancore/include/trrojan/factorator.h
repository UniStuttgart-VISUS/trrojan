/// <copyright file="factorator.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <iterator>

#include "trrojan/factor.h"


namespace trrojan {

    /// <summary>
    /// This class allows for enumerating the values of a
    /// <see cref="trrojan::factor" />.
    /// </summary>
    class TRROJANCORE_API factorator
        : public std::iterator<std::forward_iterator_tag, const variant> {

    private:

        typedef std::iterator<std::forward_iterator_tag, const variant> base;

    public:

        /// <summary>
        /// Type to measure difference between iterator positions.
        /// <summary>
        typedef base::difference_type difference_type;

        /// <summary>
        /// The category of the iterator.
        /// <summary>
        typedef base::iterator_category iterator_category;

        /// <summary>
        /// Pointer to a sequence member.
        /// <summary>
        typedef base::pointer pointer;

        /// <summary>
        /// Reference to a sequence member.
        /// <summary>
        typedef base::reference reference;

        /// <summary>
        /// Type of a sequence member.
        /// </summary>
        typedef base::value_type value_type;

        inline factorator(const factor& factor, const bool end = false)
            : factor(&factor), pos(end ? factor.size() : 0) { }

        inline factorator(const factorator& rhs)
            : factor(rhs.factor), pos(rhs.pos) { }

        /// <summary>
        /// Assignment.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        inline factorator& operator =(const factorator& rhs) {
            this->factor = rhs.factor;
            this->pos = rhs.pos;
            return *this;
        }

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>true</c> if this object and <paramref name="rhs" />
        /// are equal, <c>false</c> otherwise.</returns>
        inline bool operator ==(const factorator& rhs) const {
            return ((this->pos == rhs.pos) && (this->factor == rhs.factor));
        }

        /// <summary>
        /// Test for inequality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>false</c> if this object and <paramref name="rhs" />
        /// are equal, <c>true</c> otherwise.</returns>
        inline bool operator !=(const factorator& rhs) const {
            return !(*this == rhs);
        }

        /// <summary>
        /// Dereference the iterator.
        /// </summary>
        /// <returns>The current value of the iterator.</returns>
        inline reference operator *(void) const {
            return (*this->factor)[this->pos];
        }

        /// <summary>
        /// Access the current sequence element.
        /// </summary>
        /// <returns>Pointer to the current sequence element.</returns>
        inline pointer operator ->(void) const {
            return std::addressof((*this->factor)[this->pos]);
        }

        /// <summary>
        /// Prefix increment operator.
        /// </summary>
        /// <returns><c>*this.</c></returns>
        inline factorator& operator ++(void) {
            ++this->pos;
            return *this;
        }

        /// <summary>
        /// Postfix increment operator.
        /// </summary>
        /// <returns>The previous iterator.</returns>
        inline factorator operator ++(int) {
            factorator retval(*this);
            ++(*this);
            return std::move(retval);
        }

    private:

        /// <summary>
        /// The factor to be iterated.
        /// </summary>
        /// <remarks>
        /// The object does not own this variable, but only holds a pointer.
        /// </remarks>
        const trrojan::factor *factor;

        /// <summary>
        /// The current position of the iterator.
        /// </summary>
        size_t pos;

    };
}
