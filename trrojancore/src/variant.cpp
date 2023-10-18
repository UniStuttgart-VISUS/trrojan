// <copyright file="variant.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/variant.h"

#include <cassert>
#include <cstring>
#include <memory>

#include <wchar.h>


/*
 * trrojan::detail::operator <<
 */
TRROJANCORE_API std::ostream& trrojan::detail::operator <<(std::ostream& lhs,
        const std::wstring& rhs) {
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    lhs << cvt.to_bytes(rhs);
#else /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
    for (auto c : rhs) {
        lhs << static_cast<char>(c);
    }
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
    return lhs;
}


/*
 * trrojan::detail::operator <<
 */
TRROJANCORE_API std::ostream& trrojan::detail::operator <<(std::ostream& lhs,
        const trrojan::device& rhs) {
    lhs << ((rhs != nullptr) ? rhs->name() : "null");
    return lhs;
}


/*
 * trrojan::detail::operator <<
 */
TRROJANCORE_API std::ostream& trrojan::detail::operator <<(std::ostream& lhs,
        const trrojan::environment& rhs) {
    lhs << ((rhs != nullptr) ? rhs->name() : "null");
    return lhs;
}


/*
 * trrojan::detail::operator <<
 */
TRROJANCORE_API std::ostream& trrojan::detail::operator <<(std::ostream &lhs,
        const power_collector::pointer& rhs) {
#if defined(TRROJAN_WITH_POWER_OVERWHELMING)
    lhs << ((rhs != nullptr) ? rhs->file() : "null");
#else /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
    lhs << "null";
#endif /* defined(TRROJAN_WITH_POWER_OVERWHELMING) */
    return lhs;
}


#if defined(TRROJAN_FOR_UWP)
/*
 * trrojan::detail::operator <<
 */
TRROJANCORE_API std::ostream& trrojan::detail::operator <<(std::ostream& lhs,
        const winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow>& rhs) {
    // It does not make sense logging anything but whether a valid core window
    // was set.
    lhs << static_cast<bool>(rhs);
    return lhs;
}
#endif  /* defined(TRROJAN_FOR_UWP) */


/*
 * trrojan::variant::~variant
 */
trrojan::variant::~variant(void) {
    this->clear();
}


/*
 * trrojan::variant::operator =
 */
trrojan::variant& trrojan::variant::operator =(const variant& rhs) {
    if (this != std::addressof(rhs)) {
        rhs.conditional_invoke<detail::copy_to>(*this);
    }

    return *this;
}


/*
 * trrojan::variant::operator =
 */
trrojan::variant& trrojan::variant::operator =(variant&& rhs) noexcept {
    if (this != std::addressof(rhs)) {
        rhs.conditional_invoke<detail::move_to>(*this);
        rhs.clear();
        assert(rhs.empty());
    }

    return *this;
}


/*
 * trrojan::variant::operator ==
 */
bool trrojan::variant::operator ==(const variant& rhs) const {
    bool retval = (this == std::addressof(rhs));

    if (!retval && this->is(rhs.cur_type)) {
        this->conditional_invoke<detail::is_same>(rhs, retval);
    }

    return retval;
}
