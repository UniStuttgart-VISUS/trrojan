/// <copyright file="variant.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/variant.h"

#include <cassert>
#include <cstring>
#include <memory>

#include <wchar.h>


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
trrojan::variant& trrojan::variant::operator =(variant&& rhs) {
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
