/// <copyright file="factor_enum.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <iterator>
#include <vector>

#include "trrojan/export.h"
#include "factor_base.h"



namespace trrojan {
namespace detail {

    /// <summary>
    /// Implements a factor with discrete manifestations that are explicitly
    /// enumerated.
    /// </summary>
    class TRROJANCORE_API factor_enum : public factor_base {

    public:

        /// <summary>
        /// Initialises an empty instance.
        /// </summary>
        inline factor_enum(void) { }

        /// <summary>
        /// Initialises a new instance from a list of given manifestations.
        /// </summary>
        inline factor_enum(const std::string& name,
            const std::vector<trrojan::variant>& manifestations)
            : factor_base(name), manifestations(manifestations) { }

        /// <summary>
        /// Initialises a new instance with a list of given manifestations.
        /// </summary>
        inline factor_enum(const std::string& name,
            std::vector<trrojan::variant>&& manifestations)
            : factor_base(name), manifestations(std::move(manifestations)) { }

        /// <summary>
        /// Initialises a new instance from a range of given manifestations.
        /// </summary>
        template<class I> inline factor_enum(const std::string& name,
                I begin, I end)  : factor_base(name) {
            std::copy(begin, end, std::back_inserter(this->manifestations));
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~factor_enum(void);

        /// <inheritdoc />
        virtual std::unique_ptr<factor_base> clone(void) const;

        /// <inheritdoc />
        virtual size_t size(void) const;

        /// <inheritdoc />
        virtual trrojan::variant operator [](const size_t i) const;

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>true</c> if this object and <paramref name="rhs" />
        /// are equal, <c>false</c> otherwise.</returns>
        inline bool operator ==(const factor_enum& rhs) const {
            return (factor_base::operator ==(rhs)
                && (this->manifestations == rhs.manifestations));
        }

        /// <summary>
        /// Test for inequality.
        /// </summary>
        /// <param name="rhs">The right-hand side operand.</param>
        /// <returns><c>false</c> if this object and <paramref name="rhs" />
        /// are equal, <c>true</c> otherwise.</returns>
        inline bool operator !=(const factor_enum& rhs) const {
            return !(*this == rhs);
        }

    private:

        /// <summary>
        /// Stores all known manifestations of the factor.
        /// </summary>
        std::vector<trrojan::variant> manifestations;

    };
}
}
