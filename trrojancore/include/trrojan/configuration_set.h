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
    /// A configuration is a collection of factors which define all
    /// configurations to be tested.
    /// </summary>
    class TRROJANCORE_API configuration_set {

    public:

        /// <summary>
        /// Add an additional factor to be tested.
        /// </summary>
        void add_factor(const factor& factor);

        /// <summary>
        /// Answer whether the configuration set contains a factor with the
        /// specified name.
        /// </summary>
        /// <param name="name" />The name of the factor to be tested.</param>
        /// <returns><c>true</c> if the configuration contains a factor with the
        /// given name, <c>false</c> otherwise.</returns>
        inline bool contains_factor(const std::string& name) const {
            auto it = std::find_if(this->factors.cbegin(), this->factors.cend(),
                [&name](const factor& f) { return (f.name() == name); });
            return (it != this->factors.cend());
        }

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
        /// Gets the <see cref="trrojan::factor" />s defining the
        /// configurations.
        /// </summary>
        inline const std::vector<factor>& get_factors(void) const {
            return this->factors;
        }

    private:

        /// <summary>
        /// Holds all the factors defining the configurations.
        /// </summary>
        std::vector<factor> factors;
    };
}
