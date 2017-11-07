/// <copyright file="configuration_set.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "trrojan/configuration.h"
#include "trrojan/export.h"
#include "trrojan/factor.h"


namespace trrojan {

    /// <summary>
    /// A configuration set is a collection of factors which define all
    /// configurations to be tested.
    /// </summary>
    /// <remarks>
    /// A configuration set can be expanded into a set of
    /// <see cref="trrojan::configurations" />, which is the power set of the
    /// manifestations of all <see cref="trrrojan::factor" />s in the
    /// configuration set. The resulting configurations can be passed to one
    /// or more benchmarks to perfom the test.
    /// </remarks>
    class TRROJANCORE_API configuration_set {

    public:

        /// <summary>
        /// A list of <see cref="trrrojan::factor" />s.
        /// </summary>
        typedef std::vector<factor> factor_list;

        /// <summary>
        /// Add an additional factor to be tested.
        /// </summary>
        /// <param name="factor">The factor to be added.</param>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="factor" /> has no manifestation or if a factor with
        /// the same name is already part of the configuration set.</exception>
        void add_factor(const factor& factor);

        /// <summary>
        /// Add an additional factor. If the facor already exsits, replace the existing one.
        /// </summary>
        /// <param name="factor">The factor to be added or replacing an existing
        /// one.</param>
        /// <exception cref="std::invalid_argument">If
        /// <paramref name="factor" /> has no manifestation.</exception>
        void replace_factor(const factor& factor);

        /// <summary>
        /// Answer whether the configuration set contains a factor with the
        /// specified name.
        /// </summary>
        /// <param name="name" />The name of the factor to be tested.</param>
        /// <returns><c>true</c> if the configuration contains a factor with the
        /// given name, <c>false</c> otherwise.</returns>
        inline bool contains_factor(const std::string& name) const {
            return (this->findFactor(name) != this->_factors.cend());
        }

        /// <summary>
        /// Gets the <see cref="trrojan::factor" />s defining the
        /// configurations.
        /// </summary>
        inline const factor_list& factors(void) const {
            return this->_factors;
        }

        /// <summary>
        /// Answer the factor with the specified name or <c>nullptr</c> if no
        /// such factor is in the configuration set.
        /// </summary>
        /// <param name="name" />The name of the factor to be foud.</param>
        /// <returns>A pointer to the factor or <c>nullptr</c> if the factor is
        /// not part of the configuration set. The callee remains owner of the
        /// object being returned. Make sure not to use the pointer after
        /// changing the configuration set.</returns>
        const trrojan::factor *find_factor(const std::string& name) const;

        /// <summary>
        /// Call <paramref name="cb" /> for each configuration in the set.
        /// </summary>
        /// <remarks>
        /// <paramref name="cb" /> will be called until the last configuration
        /// is reached or until the first invocation returns <c>false</c>
        /// </remarks>
        bool foreach_configuration(
            std::function<bool(const configuration&)> cb) const;

        /// <summary>
        /// Merge <paramref name="other" /> into this configuration set.
        /// </summary>
        /// <remarks>
        /// If <paramref name="overwrite" /> is <c>true</c>,
        /// <see cref="trrojan::factor" />s in this configuration set will be
        /// overwritten by the ones from <paramref name="other" />. Otherwise, 
        /// already existing factors will be ignored.
        /// </remarks>
        /// <param name="other">The configuration set to be integrated into this
        /// one.</param>
        /// <param name="overwrite">If <c>true</c>, factors from
        /// <param name="other" /> take precedence, otherwise, factors from
        /// this configuration set will be kept unmodified.</param>
        void merge(const configuration_set& other, const bool overwrite);

        /// <summary>
        /// Reorder the factors in the configuration set such that the ones in
        /// <paramref name="factors" /> are at the beginning.
        /// </summary>
        /// <remarks>
        /// <para>When calling <see cref="foreach_configuration" />, the factors
        /// at the beginning of the list will be switched at the lowest rated.
        /// It is therefore desirable to have factors which of the change is
        /// computationally expensive are at the beginning of the list, which
        /// can be achieved with this method.</para>
        /// <para>Names of factors which are not in the configuration set will
        /// be silently ignored.</para>
        /// <para>The implementation of the method itself is not very efficient,
        /// wherefore it should be called only once before using the
        /// configuration set.</para>
        /// </remarks>
        /// <param name="factors">The names of the factors at the begin of the
        /// list.</param>.
        void optimise_order(const std::vector<std::string>& factors);

    private:

        inline factor_list::iterator findFactor(const std::string& name) {
            return std::find_if(this->_factors.begin(), this->_factors.end(),
                [&name](const factor& f) { return (f.name() == name); });
        }

        inline factor_list::const_iterator findFactor(
                const std::string& name) const {
            return std::find_if(this->_factors.cbegin(), this->_factors.cend(),
                [&name](const factor& f) { return (f.name() == name); });
        }

        /// <summary>
        /// Holds all the factors defining the configurations.
        /// </summary>
        factor_list _factors;
    };
}
