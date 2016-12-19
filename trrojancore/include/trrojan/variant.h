/// <copyright file="variant.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// Possible data types <see cref="trrojan::variant" /> can hold.
    /// </summary>
    enum class TRROJANCORE_API variant_type {
        /// <summary>
        /// The variant does not contain valid data.
        /// </summary>
        empty,

        /// <summary>
        /// The variant holds a signed 8-bit integer.
        /// </summary>
        int8,

        /// <summary>
        /// The variant holds a signed 16-bit integer.
        /// </summary>
        int16,

        /// <summary>
        /// The variant holds a signed 32-bit integer.
        /// </summary>
        int32,

        /// <summary>
        /// The variant holds a signed 64-bit integer.
        /// </summary>
        int64,

        /// <summary>
        /// The variant holds an unsigned 8-bit integer.
        /// </summary>
        uint8,

        /// <summary>
        /// The variant holds an unsigned 16-bit integer.
        /// </summary>
        uint16,

        /// <summary>
        /// The variant holds an unsigned 32-bit integer.
        /// </summary>
        uint32,

        /// <summary>
        /// The variant holds an unsigned 64-bit integer.
        /// </summary>
        uint64,

        /// <summary>
        /// The variant holds 32-bit floating point number.
        /// </summary>
        float32,

        /// <summary>
        /// The variant holds 64-bit floating point number.
        /// </summary>
        float64,

        /// <summary>
        /// The variant holds a narrow string.
        /// </summary>
        string,

        /// <summary>
        /// The variant holds a wide string.
        /// </summary>
        wstring
    };


    /// <summary>
    /// This structure allows for deriving the value type from a given
    /// <see cref="trrojan::variant_type" />.
    /// </summary>
    /// <remarks>
    /// The default implementation does indicate an invalid type; the template
    /// specialisations implement the type deduction.
    /// </remarks>
    template<variant_type T> struct TRROJANCORE_API variant_type_traits { };

#define __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(t0, t1)                         \
    template<> struct TRROJANCORE_API variant_type_traits<variant_type::t0> {  \
        typedef t1 type;                                                       \
    }                                                                          \

    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(int8, std::int8_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(int16, std::int16_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(int32, std::int32_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(int64, std::int64_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(uint8, std::uint8_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(uint16, std::uint16_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(uint32, std::uint32_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(uint64, std::uint64_t);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(float32, float);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(float64, double);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(string, char *);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(wstring, wchar_t *);

#undef __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS


    /// <summary>
    /// A type that can opaquely store different kind of data.
    /// </summary>
    class TRROJANCORE_API variant {

    public:

        /// <summary>
        /// Initialises a new instance of the class.
        /// </summary>
        inline variant(void) : cur_type(variant_type::empty), val_int64(0) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be cloned.</param>
        inline variant(const variant& rhs)
                : cur_type(variant_type::empty), val_int64(0) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        inline variant(variant&& rhs)
                : cur_type(variant_type::empty), val_int64(0) {
            *this = std::move(rhs);
        }

#define __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(t) inline variant(            \
            const variant_type_traits<variant_type::t>::type val)              \
            : cur_type(variant_type::t), val_##t(val) { }

        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(int8);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(int16);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(int32);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(int64);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(uint8);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(uint16);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(uint32);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(uint64);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(float32);
        __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR(float64);

#undef __TRROJANCORE_IMPL_INTEGRAL_VARIANT_CTOR

        /// <summary>
        /// Initialises a new instance with a copy of <paramref name="val" />.
        /// </summary>
        /// <param name="val">The initial value of the variant. It is safe to
        /// pass <c>nullptr</c>.</param>
        inline variant(const char *val) { this->set(val); }

        /// <summary>
        /// Initialises a new instance with a copy of <paramref name="val" />.
        /// </summary>
        /// <param name="val">The initial value of the variant. It is safe to
        /// pass <c>nullptr</c>.</param>
        inline variant(const wchar_t *val) { this->set(val); }

        /// <summary>
        /// Initialises a new instance with a copy of <paramref name="val" />.
        /// </summary>
        /// <param name="val">The initial value of the variant.</param>
        inline variant(const std::string& val) { this->set(val.c_str()); }

        /// <summary>
        /// Initialises a new instance with a copy of <paramref name="val" />.
        /// </summary>
        /// <param name="val">The initial value of the variant.</param>
        inline variant(const std::wstring& val) { this->set(val.c_str()); }

        /// <summary>
        /// Answer whether the variant is empty.
        /// </summary>
        /// <returns><c>true</c> if the variant is empty,
        /// <c>false</c> otherwise.
        inline bool empty(void) const {
            return (this->cur_type == variant_type::empty);
        }

#define __TRROJANCORE_IMPL_GET_VARIANT(t)                                      \
        inline const typename variant_type_traits<variant_type::t>::type       \
        get_##t(void) const {                                                  \
            if (!this->is(variant_type::t)) {                                  \
                throw std::logic_error("variant does not hold this type.");    \
            }                                                                  \
            return this->val_##t;                                              \
        }

        __TRROJANCORE_IMPL_GET_VARIANT(int8);
        __TRROJANCORE_IMPL_GET_VARIANT(int16);
        __TRROJANCORE_IMPL_GET_VARIANT(int32);
        __TRROJANCORE_IMPL_GET_VARIANT(int64);
        __TRROJANCORE_IMPL_GET_VARIANT(uint8);
        __TRROJANCORE_IMPL_GET_VARIANT(uint16);
        __TRROJANCORE_IMPL_GET_VARIANT(uint32);
        __TRROJANCORE_IMPL_GET_VARIANT(uint64);
        __TRROJANCORE_IMPL_GET_VARIANT(float32);
        __TRROJANCORE_IMPL_GET_VARIANT(float64);
        __TRROJANCORE_IMPL_GET_VARIANT(string);
        __TRROJANCORE_IMPL_GET_VARIANT(wstring);

#undef __TRROJANCORE_IMPL_GET_VARIANT

        /// <summary>
        /// Answer whether the variant holds the given type.
        /// </summary>
        /// <param name="type">The type to be tested</param>
        /// <returns><c>true</c> if the variants holds a value of
        /// <paramref name="type" />, <c>false</c> otherwise.</returns>
        inline bool is(const variant_type type) const {
            return (this->cur_type == type);
        }

#define __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(t) inline void set_##t(         \
                const typename variant_type_traits<variant_type::t>::type val) {\
            this->clean_before_set();                                           \
            this->cur_type = variant_type::t;                                   \
            this->val_##t = val;                                                \
        }

        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int8);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int16);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int64);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint8);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint16);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint64);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(float32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(float64);

#undef __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT

        /// <summary>
        /// Assigns a clone of <paramref name="val" /> as new value of the
        /// variant.
        /// </summary>
        /// <param name="val">The new value of the variant. It is safe to
        /// pass <c>nullptr</c>.</param>
        inline void set_string(const char *val) {
            this->clean_before_set();
            this->set(val);
        }

        /// <summary>
        /// Assigns a clone of <paramref name="val" /> as new value of the
        /// variant.
        /// </summary>
        /// <param name="val">The new value of the variant.</param>
        inline void set_string(const std::string& val) {
            this->set(val.c_str());
        }

        /// <summary>
        /// Assigns a clone of <paramref name="val" /> as new value of the
        /// variant.
        /// </summary>
        /// <param name="val">The new value of the variant. It is safe to
        /// pass <c>nullptr</c>.</param>
        inline void set_wstring(const wchar_t *val) {
            this->clean_before_set();
            this->set(val);
        }

        /// <summary>
        /// Assigns a clone of <paramref name="val" /> as new value of the
        /// variant.
        /// </summary>
        /// <param name="val">The new value of the variant.</param>
        inline void set_wstring(const std::wstring& val) {
            this->set(val.c_str());
        }

        /// <summary>
        /// Answer the current type of the variant.
        /// </summary>
        /// <returns>The current type of the variant</returns>
        inline variant_type type(void) const {
            return this->cur_type;
        }

#define __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(t) inline variant& operator =(  \
                const typename variant_type_traits<variant_type::t>::type rhs) {\
            this->clean_before_set();                                           \
            this->cur_type = variant_type::t;                                   \
            this->val_##t = rhs;                                                \
            return *this;                                                       \
        }

        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int8);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int16);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(int64);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint8);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint16);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(uint64);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(float32);
        __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT(float64);

#undef __TRROJANCORE_IMPL_SET_INTEGRAL_VARIANT

        /// <summary>
        /// Assignment from string.
        /// </summary>
        /// <remarks>
        /// The string will be deep copied.
        /// </remarks>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        inline variant& operator =(const char *rhs) {
            this->clean_before_set();
            this->set(rhs);
            return *this;
        }

        /// <summary>
        /// Assignment from string.
        /// </summary>
        /// <remarks>
        /// The string will be deep copied.
        /// </remarks>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        inline variant& operator =(const wchar_t *rhs) {
            this->clean_before_set();
            this->set(rhs);
            return *this;
        }

        /// <summary>
        /// Assignment from string.
        /// </summary>
        /// <remarks>
        /// The string will be deep copied.
        /// </remarks>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        inline variant& operator =(const std::string& rhs) {
            return (*this = rhs.c_str());
        }

        /// <summary>
        /// Assignment from string.
        /// </summary>
        /// <remarks>
        /// The string will be deep copied.
        /// </remarks>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        inline variant& operator =(const std::wstring& rhs) {
            return (*this = rhs.c_str());
        }

        /// <summary>
        /// Assignment.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        variant& operator =(const variant& rhs);

        /// <summary>
        /// Move assignment.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        variant& operator =(variant&& rhs);

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if this variant and <paramref name="rhs" />
        /// are equal.</returns>
        bool operator ==(const variant& rhs) const;

        /// <summary>
        /// Test for equality.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if this variant and <paramref name="rhs" />
        /// are equal.</returns>
        inline bool operator !=(const variant& rhs) const {
            return !(*this == rhs);
        }

        /// <summary>
        /// Write the <see cref="trrojan::variant" /> to a stream.
        /// </summary>
        /// <param name="lhs"></param>
        /// <param name="rhs"></param>
        /// <returns><paramref name="lhs" /></returns>
        /// <tparam name="C"></tparam>
        /// <tparam name="T"></tparam>
        template<class C, class T>
        friend inline std::basic_ostream<C, T>& operator <<(
                std::basic_ostream<C, T>& lhs, const variant& rhs) {
#define __TRROJANCORE_PRINT(t)                                                 \
                case variant_type::t: lhs << rhs.val_##t; return lhs

            switch (rhs.cur_type) {
                __TRROJANCORE_PRINT(int8);
                __TRROJANCORE_PRINT(int16);
                __TRROJANCORE_PRINT(int32);
                __TRROJANCORE_PRINT(int64);
                __TRROJANCORE_PRINT(uint8);
                __TRROJANCORE_PRINT(uint16);
                __TRROJANCORE_PRINT(uint32);
                __TRROJANCORE_PRINT(uint64);
                __TRROJANCORE_PRINT(float32);
                __TRROJANCORE_PRINT(float64);
                __TRROJANCORE_PRINT(string);
                __TRROJANCORE_PRINT(wstring);
                default: return lhs;
            }

#undef __TRROJANCORE_PRINT
        }

    private:

        void clean_before_set(void);

        void set(const char *val);

        void set(const wchar_t *val);

        variant_type cur_type;

        union {
            std::int8_t val_int8;
            std::int16_t val_int16;
            std::int32_t val_int32;
            std::int64_t val_int64;
            std::uint8_t val_uint8;
            std::uint16_t val_uint16;
            std::uint32_t val_uint32;
            std::uint64_t val_uint64;
            float val_float32;
            double val_float64;
            char *val_string;
            wchar_t *val_wstring;
        };
    };
}
