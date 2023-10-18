// <copyright file="winrt_traits.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <type_traits>

#if defined(TRROJAN_FOR_UWP)
#include <winrt/windows.ui.core.h>
#endif /* defined(TRROJAN_FOR_UWP) */


namespace trrojan {
namespace detail {

#if defined(TRROJAN_FOR_UWP)
    /// <summary>
    /// A little helper for finding out whether <typeparamref name="T" /> is
    /// derived from the WinRT unknown interface.
    /// </summary>
    /// <typeparam name="T">The type of a WinRT object that ish to be
    /// checked whether it is a child of
    /// <see cref="winrt::Windows::Foundation::IUnknown" />.</typeparam>
    template<class T> using is_winrt_unknown = std::is_base_of<
        winrt::Windows::Foundation::IUnknown, T>;

    /// <summary>
    /// Enable comparison of <see cref="winrt::agile_ref" />s, which is required
    /// for the <see cref="variant" /> to work.
    /// </summary>
    /// <typeparam name="T">The object that is being referenced, which must be
    /// derived from <see cref="winrt::Windows::Foundation::IUnknown" />.
    /// </typeparam>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    template<class T>
    typename std::enable_if<is_winrt_unknown<T>::value, bool>::type
    operator == (const winrt::agile_ref<T>& lhs,
            const winrt::agile_ref<T>& rhs) {
        auto l = lhs.get();
        auto r = rhs.get();
        return (l == r);
    }

    /// <summary>
    /// Enable comparison of <see cref="winrt::agile_ref" />s, which is required
    /// for the <see cref="variant" /> to work.
    /// </summary>
    /// <typeparam name="T">The object that is being referenced, which
    /// must be a instance of <see cref="winrt::com_ptr" />.</typeparam>
    /// <param name="lhs"></param>
    /// <param name="rhs"></param>
    /// <returns></returns>
    template<class T>
    typename std::enable_if<!is_winrt_unknown<T>::value, bool>::type
    operator == (const winrt::agile_ref<T>& lhs,
            const winrt::agile_ref<T>& rhs) {
        auto l = lhs.get();
        auto r = rhs.get();
        return (l.get() == r.get());
    }
#endif /* defined(TRROJAN_FOR_UWP) */

} /* namespace detail */
} /* namespace trrojan */
