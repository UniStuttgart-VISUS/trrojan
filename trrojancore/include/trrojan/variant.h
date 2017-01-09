/// <copyright file="variant.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <cassert>
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
#include <codecvt>
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
#include <cstdint>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "trrojan/device.h"
#include "trrojan/environment.h"
#include "trrojan/export.h"


namespace trrojan {

    /* Forward declarations */
    class TRROJANCORE_API variant;


    /// <summary>
    /// Possible data types <see cref="trrojan::variant" /> can hold.
    /// </summary>
    /// <remarks>
    /// <para>If a new type of data to be stored in the variant is added here,
    /// the following things need to be updated, too:
    /// <list type="bullet">
    /// <item><description>A new member named "val_[membername]" must be added
    /// in <see cref="trrojan::detail::variant" /> below.</description></item>
    /// <item><description>A new line of the 
    /// <c>__TRROJANCORE_DECL_VARIANT_TYPE_TRAITS</c> declarations must be
    /// added below.</description></item>
    /// <item><description>Add the new member to the variadic template list of
    /// the <see cref="trrojan::detail::variant_type_list" /> <c>typedef</c>
    /// below to enable conditional invocation.
    /// </description></item>
    /// </list>
    /// </para>
    /// <para>Please be aware that all types in the variant must be
    /// default-constructible</para>
    /// </remarks>
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
        wstring,

        /// <summary>
        /// The variant holds a <see cref="trrojan::device" />.
        /// </summary>
        device,

        /// <summary>
        /// The variant holds a <see cref="trrojan::environment" />.
        /// </summary>
        environment

        // Add new members here.
    };


namespace detail {
    /// <summary>
    //// Actual storage structure for <see cref="trrojan::variant" />.
    /// </summary>
    /// <remarks>
    /// <para>This union should never be used except for in 
    /// <see cref="trrojan::variant" />!</para>
    /// <para>The union defines an empty default constructor and an empty
    /// destructor, which results in its members being treated like
    /// uninitialised memory. The <see cref="trrojan::variant" /> must handle
    /// initialisation and finalisation on behalf of the union.
    /// </remarks>
    union variant {
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
        std::string val_string;
        std::wstring val_wstring;
        device val_device;
        environment val_environment;
        // Add new members here.

        inline variant(void) { }
        inline ~variant(void) { }
    };


    /// <summary>
    /// A list of variant types to expand.
    /// </summary>
    template<trrojan::variant_type...> struct variant_type_list_t { };

    /// <summary>
    /// The actual type list we use in 
    /// <see cref="trrojan::variant::conditional_invoke" />.
    /// </summary>
    /// <remarks>
    /// New types in <see cref="trrojan::variant_type" /> must be added here.
    /// </remarks>
    typedef variant_type_list_t<variant_type::int8, variant_type::int16,
        variant_type::int32, variant_type::int64, variant_type::uint8,
        variant_type::uint16, variant_type::uint32, variant_type::uint64,
        variant_type::float32, variant_type::float64, variant_type::string,
        variant_type::wstring, variant_type::device, variant_type::environment
        /* Add new members here. */>
        variant_type_list;

} /* end namespace detail */


    /// <summary>
    /// This structure allows for deriving the value type from a given
    /// <see cref="trrojan::variant_type" />.
    /// </summary>
    /// <remarks>
    /// The default implementation does indicate an invalid type; the template
    /// specialisations implement the type deduction.
    /// </remarks>
    template<variant_type T> struct TRROJANCORE_API variant_type_traits { };

    /// <summary>
    /// This structure allows for deriving the
    /// <see cref="trrojan::variant_type" /> from a C++ type.
    /// </summary>
    /// <remarks>
    /// <para>This structure implements the inverse operation of
    /// <see cref="trrojan::variant_type_traits" />.</para>
    /// <para>The default implementation does indicate an invalid type; the
    /// template specialisations implement the type deduction.</para>
    /// </remarks>
    template<class T> struct TRROJANCORE_API variant_reverse_traits { };

#define __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(t0, t1)                         \
    template<> struct TRROJANCORE_API variant_type_traits<variant_type::t0> {  \
        typedef t1 type;                                                       \
        inline static t1 *get(detail::variant& v) {                            \
            return &v.val_##t0;                                                \
        }                                                                      \
        inline static const t1 *get(const detail::variant& v) {                \
            return &v.val_##t0;                                                \
        }                                                                      \
    };                                                                         \
    template<> struct TRROJANCORE_API variant_reverse_traits<t1> {             \
        static const variant_type type = variant_type::t0;                     \
        inline static t1 *get(detail::variant& v) {                            \
            return &v.val_##t0;                                                \
        }                                                                      \
        inline static const t1 *get(const detail::variant& v) {                \
            return &v.val_##t0;                                                \
        }                                                                      \
    }

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
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(string, std::string);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(wstring, std::wstring);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(device, trrojan::device);
    __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS(environment, trrojan::environment);
    // Add new specialisations here here.

#undef __TRROJANCORE_DECL_VARIANT_TYPE_TRAITS


namespace detail {
    /// <summary>
    /// Functor which tries casting the value of the variant to a specific
    /// type and returning it.
    /// </summary>
    template<variant_type T> struct cast_to {
        typedef typename variant_type_traits<T>::type type;

        template<class U>
        static inline typename std::enable_if<std::is_convertible<type, U>::value>::type
        invoke(type& v, U& target) {
            target = static_cast<U>(v);
        }

        template<class U>
        static inline typename std::enable_if<!std::is_convertible<type, U>::value>::type
        invoke(type& v, U& target) {
            throw std::bad_cast();
        }
    };

    /// <summary>
    /// Functor which copies the data of the calling variant to another one.
    /// </summary>
    template<variant_type T> struct copy_to {
        typedef typename variant_type_traits<T>::type type;
        static void invoke(type& v, trrojan::variant& target);
    };

    /// <summary>
    /// Functor which destructs the currently active variant data.
    /// </summary>
    template<variant_type T> struct destruct {
        typedef typename variant_type_traits<T>::type type;
        static inline void invoke(type& v) { v.~type(); }
    };

    /// <summary>
    /// Functor which compares the values of two variants with the same
    /// type.
    /// </summary>
    template<variant_type T> struct is_same {
        typedef typename variant_type_traits<T>::type type;
        static void invoke(type& v, const trrojan::variant& rhs, bool& retval);
    };

    /// <summary>
    /// Functor which moves the data of the calling variant to another
    /// one.
    /// </summary>
    /// <remarks>
    /// Please not that the source variant will be unmodified, only the
    /// data are moved, ie changes to the state of the variant itself must
    /// be done manually after the call.
    /// </remarks>
    template<variant_type T> struct move_to {
        typedef typename variant_type_traits<T>::type type;
        static void invoke(type& v, trrojan::variant& target);
    };

    /// <summary>
    /// Functor for printing the currently active variant data to an
    /// <see cref="std::ostream" />.
    /// </summary>
    template<variant_type T> struct print {
        typedef typename variant_type_traits<T>::type type;
        static inline void invoke(type& v, std::ostream& stream) {
            stream << v;
        }
    };

    /// <summary>
    /// Specialisation of the <see cref="variant::print" /> functor for
    /// <see cref="std::wstring" />, which performs the necessary string
    /// conversion before output.
    /// </summary>
    template<> struct print<variant_type::wstring> {
        static inline void invoke(std::wstring& v, std::ostream& stream) {
#if (!defined(__GNUC__) || (__GNUC__ >= 5))
            static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
            stream << cvt.to_bytes(v);
#else /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
            for (auto c : v) {
                stream << static_cast<char>(c);
            }
#endif /* (!defined(__GNUC__) || (__GNUC__ >= 5)) */
        }
    };

    /// <summary>
    /// Specialisation of the <see cref="variant::print" /> functor for
    /// <see cref="trrojan::environment" />, which prints the environemnt name.
    /// </summary>
    template<> struct print<variant_type::environment> {
        static inline void invoke(trrojan::environment& v,
                std::ostream& stream) {
            stream << ((v != nullptr) ? v->name() : "null");
        }
    };
} /* end namespace detail */


    /// <summary>
    /// A type that can opaquely store different kind of data.
    /// </summary>
    class TRROJANCORE_API variant {

    public:

        /// <summary>
        /// Initialises a new instance of the class.
        /// </summary>
        inline variant(void) : cur_type(variant_type::empty) { }

        /// <summary>
        /// Clone <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be cloned.</param>
        inline variant(const variant& rhs) : cur_type(variant_type::empty) {
            *this = rhs;
        }

        /// <summary>
        /// Move <paramref name="rhs" />.
        /// </summary>
        /// <param name="rhs">The object to be moved.</param>
        inline variant(variant&& rhs) : cur_type(variant_type::empty) {
            *this = std::move(rhs);
        }

        /// <summary>
        /// Initialise from <paramref name="value" />.
        /// </summary>
        /// <remarks>
        /// This constructor allows for implicit conversions from
        /// <tparamref name="T" />.
        /// </remarks>
        /// <param name="value">The initial value of the variant.</param>
        template<class T>
        inline variant(const T value) : cur_type(variant_type::empty) {
            this->set<T>(value);
        }

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~variant(void);

        /// <summary>
        /// Converts the value of the variant to <tparamref name="T" /> if
        /// possible.
        /// </summary>
        /// <tparam name="T">The target type.</tparam>
        /// <returns>The value of the variant casted to <tparamref name="T" />.
        /// </returns>
        /// <exception cref="std::bad_cast">If the currently stored value is not
        /// convertible to <tparam name="T" />.</exception>
        template<class T> inline T as(void) const {
            auto retval = T();
            this->conditional_invoke<detail::cast_to>(retval);
            return retval;
        }

        /// <summary>
        /// Clears the variant, effectively resetting it to empty state.
        /// </summary>
        inline void clear(void) {
            this->conditional_invoke<detail::destruct>();
            this->cur_type = variant_type::empty;
        }

        /// <summary>
        /// Answer whether the variant is empty.
        /// </summary>
        /// <returns><c>true</c> if the variant is empty,
        /// <c>false</c> otherwise.
        inline bool empty(void) const {
            return (this->cur_type == variant_type::empty);
        }

        /// <summary>
        /// Gets the value of the variant interpreted as type
        /// <tparamref name="T" />.
        /// </summary>
        /// <remarks>
        /// The method does not check (except for an assertion) whether it is
        /// legal to retrieve the value as the specified type. It is up to the
        /// user to check this beforehand.
        /// </remarks>
        /// <tparam name="T">The member of <see cref="trrojan::variant_type" />
        /// matching the value currently returned by
        /// <see cref="trrojan::variant::type" />.</tparam>
        /// <returns>The current value of the variant.</returns>
        template<variant_type T>
        const typename variant_type_traits<T>::type& get(void) const {
            assert(this->cur_type == T);
            return *variant_type_traits<T>::get(this->data);
        }

        /// <summary>
        /// Gets the value of the variant interpreted as type
        /// <tparamref name="T" />.
        /// </summary>
        /// <remarks>
        /// The method does not check (except for an assertion) whether it is
        /// legal to retrieve the value as the specified type. It is up to the
        /// user to check this beforehand.
        /// </remarks>
        /// <tparam name="T">The C++ currently stored in the variant.</tparam>
        /// <returns>The current value of the variant.</returns>
        template<class T> const T& get(void) const {
            assert(this->cur_type == variant_reverse_traits<T>::type);
            return *variant_reverse_traits<T>::get(this->data);
        }

        /// <summary>
        /// Answer whether the variant holds the given type.
        /// </summary>
        /// <param name="type">The type to be tested</param>
        /// <returns><c>true</c> if the variants holds a value of
        /// <paramref name="type" />, <c>false</c> otherwise.</returns>
        inline bool is(const variant_type type) const {
            return (this->cur_type == type);
        }

        /// <summary>
        /// Sets a new value.
        /// </summary>
        /// <param name="value">The new value of the variant.</param>
        /// <tparam name="T">The new type of the variant.</tparam>
        template<variant_type T>
        void set(const typename variant_type_traits<T>::type& value) {
            this->reconstruct<T>();
            *variant_type_traits<T>::get(this->data) = value;
        }

        /// <summary>
        /// Sets a new value.
        /// </summary>
        /// <param name="value">The new value of the variant.</param>
        /// <tparam name="T">The type of the new value.</tparam>
        template<class T> void set(const T& value) {
            typedef typename std::decay<T>::type type;
            this->reconstruct<variant_reverse_traits<type>::type>();
            *variant_reverse_traits<type>::get(this->data) = value;
        }

        /// <summary>
        /// Sets a new value by moving the data.
        /// </summary>
        /// <param name="value">The new value of the variant.</param>
        /// <tparam name="T">The new type of the variant.</tparam>
        template<variant_type T>
        void set(typename variant_type_traits<T>::type&& value) {
            this->reconstruct<T>();
            *variant_type_traits<T>::get(this->data) = std::move(value);
        }

        /// <summary>
        /// Sets a new value by moving the data.
        /// </summary>
        /// <param name="value">The new value of the variant.</param>
        /// <tparam name="T">The type of the new value.</tparam>
        template<class T> void set(T&& value) {
            typedef typename std::decay<T>::type type;
            this->reconstruct<variant_reverse_traits<type>::type>();
            *variant_reverse_traits<type>::get(this->data) = std::move(value);
        }

        /// <summary>
        /// Answer the current type of the variant.
        /// </summary>
        /// <returns>The current type of the variant</returns>
        inline variant_type type(void) const {
            return this->cur_type;
        }

        /// <summary>
        /// Assignment.
        /// </summary>
        /// <remarks>
        /// This operator is an alias for <see cref="trrojan::variant::set" />.
        /// </remarks>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>*this</c></returns>
        template<variant_type T> inline variant& operator =(
                const typename variant_type_traits<T>::type& rhs) const {
            this->set(rhs);
            return *this;
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
        /// Test for inequality.
        /// </summary>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if this variant and <paramref name="rhs" />
        /// are not equal.</returns>
        inline bool operator !=(const variant& rhs) const {
            return !(*this == rhs);
        }

        /// <summary>
        /// Cast to <tparamref name="T" />.
        /// </summary>
        /// <remarks>
        /// This operator is an alias for <see cref="trrojan::variant::as" />.
        /// </remarks>
        /// <tparam name="T">The target type.</tparam>
        /// <returns>The value of the variant casted to <tparamref name="T" />.
        /// </returns>
        /// <exception cref="std::bad_cast">If the currently stored value is not
        /// convertible to <tparam name="T" />.</exception>

        template<class T> operator T(void) const {
            return this->as<T>();
        }

        /// <summary>
        /// Write the <see cref="trrojan::variant" /> to a stream.
        /// </summary>
        /// <param name="lhs">The left-hand side operand (the stream to
        /// write to).</param>
        /// <param name="rhs">The right-hand side operand (the object to
        /// be written).</param>
        /// <returns><paramref name="lhs" />.</returns>
        friend inline std::ostream& operator <<(std::ostream& lhs,
                const variant& rhs) {
            rhs.conditional_invoke<detail::print>(lhs);
            return lhs;
        }

    private:

        /// <summary>
        /// Invokes the functor <tparamref name="F" /> on the currently active
        /// variant data.
        /// </summary>
        /// <param name="params">An optional list of runtime-parameters
        /// which are forwarded to the functor.</param>
        /// <tparam name="F">The functor to be called for the matching
        /// <see cref="trrojan::variant_type" />.</tparam>
        /// <tparam name="P">The parameter list for the functor.</tparam>
        template<template<variant_type> class F, class... P>
        inline void conditional_invoke(P&&... params) {
            this->conditional_invoke0<F>(detail::variant_type_list(),
                std::forward<P>(params)...);
        }

        /// <summary>
        /// Invokes the functor <tparamref name="F" /> on the currently active
        /// variant data.
        /// </summary>
        /// <param name="params">An optional list of runtime-parameters
        /// which are forwarded to the functor.</param>
        /// <tparam name="F">The functor to be called for the matching
        /// <see cref="trrojan::variant_type" />.</tparam>
        /// <tparam name="P">The parameter list for the functor.</tparam>
        template<template<variant_type> class F, class... P>
        inline void conditional_invoke(P&&... params) const {
            // Rationale: We do not want to implement the whole stack twice, so
            // we use the const_cast here as we know that our own functors do
            // not do nasty things.
            const_cast<variant *>(this)->conditional_invoke0<F>(
                detail::variant_type_list(), std::forward<P>(params)...);
        }

        /// <summary>
        /// Invokes the functor <tparamref name="F" /> on the data of the 
        /// variant if the the variant's type is <tparamref name="T" />.
        /// </summary>
        /// <param name="params">An optional list of runtime-parameters
        /// which are forwarded to the functor.</param>
        /// <tparam name="F">The functor to be called for the matching
        /// <see cref="trrojan::variant_type" />.</tparam>
        /// <tparam name="T">The type to be currently evaluated. It this
        /// is the active type in the variant, the functor is invoked on
        /// the data.</tparam>
        /// <tparam name="U">The rest of types to be evaluated.</tparam>
        /// <tparam name="P">The parameter list for the functor.</tparam>
        template<template<variant_type> class F, variant_type T,
            variant_type... U, class... P>
        void conditional_invoke0(detail::variant_type_list_t<T, U...>,
            P&&... params);

        /// <summary>
        /// End of recursion for
        /// <see cref="trrojan::variant::conditional_invoke0" />.
        /// </summary>
        /// <param name="params">An optional list of runtime-parameters
        /// which are forwarded to the functor.</param>
        /// <tparam name="F">The functor to be called for the matching
        /// <see cref="trrojan::variant_type" />.</tparam>
        /// <tparam name="P">The parameter list for the functor.</tparam>
        template<template<variant_type> class F, class... P>
        inline void conditional_invoke0(detail::variant_type_list_t<>,
            P&&... params) { }

        /// <summary>
        /// Clears the variant and (re-) constructs it as type
        /// <tparamref name="T" />.
        /// </summary>
        /// <remarks>
        /// The effects of this method are that (i) any previous value will be
        /// destructed, (ii) the new type designated by <tparamref name="T" />
        /// will be constructed and (iii) the current type will be set to
        /// <tparamref name="T" />.
        /// <remarks>
        /// <tparam name="T">The new type of the variant, which should be
        /// initialised by the method.</tparam>
        template<variant_type T> inline void reconstruct(void) {
            typedef typename variant_type_traits<T>::type type;
            this->conditional_invoke<detail::destruct>();
            ::new (variant_type_traits<T>::get(this->data)) type();
            this->cur_type = T;
        }

        /// <summary>
        /// Stores which of the members of <see cref="data" /> is currently
        /// valid.
        /// </summary>
        variant_type cur_type;

        /// <summary>
        /// Stores the actual value of the variant.
        /// </summary>
        detail::variant data;
    };
}

#include "trrojan/variant.inl"
