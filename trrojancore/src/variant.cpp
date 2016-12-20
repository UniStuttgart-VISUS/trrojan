/// <copyright file="variant.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/variant.h"

#include <cstring>
#include <memory>

#include <wchar.h>



/*
 * trrojan::variant::~variant
 */
trrojan::variant::~variant(void) {
    this->set_uint16(0);    // Will call clean_before_set and erase any
                            // string or pointer value.
}


/*
 * trrojan::variant::operator =
 */
trrojan::variant& trrojan::variant::operator =(const variant& rhs) {
    if (this != std::addressof(rhs)) {
        this->clean_before_set();

        switch (rhs.cur_type) {
            // TODO: this is hugly
            case variant_type::device:
                this->val_device = rhs.val_device;
                break;

            case variant_type::environment:
                this->val_environment = rhs.val_environment;
                break;

            case variant_type::string:
                this->set(rhs.val_string);
                break;

            case variant_type::wstring:
                this->set(rhs.val_wstring);
                break;

            default:
                this->cur_type = rhs.cur_type;
                this->val_uint64 = rhs.val_uint64;
                break;
        }
    }

    return *this;
}


/*
 * trrojan::variant::operator =
 */
trrojan::variant& trrojan::variant::operator =(variant&& rhs) {
    if (this != std::addressof(rhs)) {
        // TODO: this is hugly
        this->clean_before_set();
        switch (rhs.cur_type) {
            // TODO: fixme
            case variant_type::device:
                this->val_device = rhs.val_device;
                break;

            case variant_type::environment:
                this->val_environment = rhs.val_environment;
                break;

            default:
                this->val_uint64 = rhs.val_uint64;
                break;
        }
        this->cur_type = rhs.cur_type;
        rhs.cur_type = variant_type::empty;
        rhs.clean_before_set();
    }

    return *this;
}


/*
 * trrojan::variant::operator ==
 */
bool trrojan::variant::operator ==(const variant& rhs) const {
    if (this->is(rhs.cur_type)) {
        // TODO: fixme
        return (this->val_uint64 == rhs.val_uint64);
    } else {
        return false;
    }
}


/*
 * trrojan::variant::clean_before_set
 */
void trrojan::variant::clean_before_set(void) {
    switch (this->cur_type) {
        case variant_type::device:
            this->val_device.reset();
            break;

        case variant_type::environment:
            this->val_environment.reset();
            break;

        case variant_type::string:
            if (this->val_string != nullptr) {
                delete[] this->val_string;
                this->val_string = nullptr;
            }
            break;

        case variant_type::wstring:
            if (this->val_wstring != nullptr) {
                delete[] this->val_wstring;
                this->val_wstring = nullptr;
            }
            break;
    }
}


/*
 * trrojan::variant::set
 */
void trrojan::variant::set(const char *val) {
    this->cur_type = variant_type::string;
    this->val_string = nullptr;

    if (val != nullptr) {
        auto len = ::strlen(val) + 1;
        this->val_string = new char[len];
#ifdef _MSC_VER
        ::strcpy_s(this->val_string, len, val);
#else /* _MSC_VER */
        ::strcpy(this->val_string, val);
#endif /* _MSC_VER */
    }
}


/*
 * trrojan::variant::set
 */
void trrojan::variant::set(const wchar_t *val) {
    this->cur_type = variant_type::wstring;
    this->val_wstring = nullptr;

    if (val != nullptr) {
        auto len = ::wcslen(val) + 1;
        this->val_wstring = new wchar_t[len];
#ifdef _MSC_VER
        ::wcscpy_s(this->val_wstring, len, val);
#else /* _MSC_VER */
        ::wcscpy(this->val_wstring, val);
#endif /* _MSC_VER */
    }
}
