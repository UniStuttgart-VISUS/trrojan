// <copyright file="tuple_cat_type" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include <tuple>


namespace trrojan {

    /// <summary>
    /// Derives the type of two concatenated tuples.
    /// </summary>
    /// <remarks>
    /// We prefer this implementation over <c>decltype</c> on <c>tuple_cat</c>,
    /// because <c>tuple_cat</c> is <c>nodiscard</c> and would raise warnings.
    /// </remarks>
    /// <typeparam name="TLeft">The left-hand side tuple.</typeparam>
    /// <typeparam name="TRight">The right-hand side tuple.</typeparam>
    template<class TLeft, class TRight> struct tuple_cat_type { };

    /// <summary>
    /// Specialisation perfoming the actual concatenation of types.
    /// </summary>
    template<class... TLeft, class... TRight>
    struct tuple_cat_type<std::tuple<TLeft...>, std::tuple<TRight...>> {

        /// <summary>
        /// The type of the concatenated tuple.
        /// </summary>
        typedef std::tuple<TLeft..., TRight...> type;
    };

} /* end namespace trrojan */
