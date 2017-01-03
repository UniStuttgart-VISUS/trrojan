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
        const_cast<variant&>(rhs).conditional_invoke<copy_to>(*this);
    }

    return *this;
}


/*
 * trrojan::variant::operator =
 */
trrojan::variant& trrojan::variant::operator =(variant&& rhs) {
    if (this != std::addressof(rhs)) {
        rhs.conditional_invoke<move_to>(*this);
        rhs.clear();
        assert(rhs.empty());
    }

    return *this;
}


/*
 * trrojan::variant::operator ==
 */
bool trrojan::variant::operator ==(const variant& rhs) const {
    if (this->is(rhs.cur_type)) {
        // TODO

        // TODO: this is hugly
        //switch (rhs.cur_type) {
        //    case variant_type::device:
        //        return (this->val_device == rhs.val_device);

        //    case variant_type::environment:
        //        return (this->val_environment == rhs.val_environment);
        //    break;

        //    default:
        //        return (this->val_uint64 == rhs.val_uint64);
        //}
        return false;
    } else {
        /* If the type is not the same, the value does not matter. */
        return false;
    }
}
