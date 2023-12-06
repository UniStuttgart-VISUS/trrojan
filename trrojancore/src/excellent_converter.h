// <copyright file="excellent_converter.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>


#if defined(_WIN32) && !defined(_UWP)
#include <sstream>

#include <atlbase.h>
#include <Ole2.h>

#include "trrojan/text.h"
#include "trrojan/variant.h"


namespace trrojan {

    /// <summary>
    /// A converter type that transforms our <see cref="variant" /> into an OLE
    /// <see cref="VARIANT" />.
    /// </summary>
    /// <typeparam name="Type"></typeparam>
    template<variant_type Type> struct excellent_converter {
        static void convert(VARIANT& dst, const variant& src) {
            std::stringstream ss;
            ss << src << std::ends;
            const auto value = from_utf8(ss.str());
            dst.vt = VT_BSTR;
            dst.bstrVal = ::SysAllocString(value.c_str());
        }
    };

    template<> struct excellent_converter<variant_type::boolean> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_BOOL;
            dst.boolVal = src.get<variant_type::boolean>();
        }
    };

    template<> struct excellent_converter<variant_type::int8> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_I1;
            dst.cVal = src.get<variant_type::int8>();
        }
    };

    template<> struct excellent_converter<variant_type::int16> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_I2;
            dst.iVal = src.get<variant_type::int16>();
        }
    };

    template<> struct excellent_converter<variant_type::int32> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_I4;
            dst.intVal = src.get<variant_type::int32>();
        }
    };

    template<> struct excellent_converter<variant_type::int64> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_I8;
            dst.llVal = src.get<variant_type::int64>();
        }
    };

    template<> struct excellent_converter<variant_type::uint8> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_UI1;
            dst.bVal = src.get<variant_type::uint8>();
        }
    };

    template<> struct excellent_converter<variant_type::uint16> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_UI2;
            dst.uiVal = src.get<variant_type::uint16>();
        }
    };

    template<> struct excellent_converter<variant_type::uint32> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_UI4;
            dst.uintVal = src.get<variant_type::uint32>();
        }
    };

    template<> struct excellent_converter<variant_type::uint64> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_UI8;
            dst.ullVal = src.get<variant_type::uint64>();
        }
    };

    template<> struct excellent_converter<variant_type::float32> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_R4;
            dst.fltVal = src.get<variant_type::float32>();
        }
    };

    template<> struct excellent_converter<variant_type::float64> {
        static void convert(VARIANT& dst, const variant& src) {
            dst.vt = VT_R8;
            dst.dblVal = src.get<variant_type::float64>();
        }
    };

    template<> struct excellent_converter<variant_type::string> {
        static void convert(VARIANT& dst, const variant& src) {
            const auto value = from_utf8(src.get<variant_type::string>());
            dst.vt = VT_BSTR;
            dst.bstrVal = ::SysAllocString(value.c_str());
        }
    };

    template<> struct excellent_converter<variant_type::wstring> {
        static void convert(VARIANT& dst, const variant& src) {
            const auto value = src.get<variant_type::wstring>();
            dst.vt = VT_BSTR;
            dst.bstrVal = ::SysAllocString(value.c_str());
        }
    };

    template<> struct excellent_converter<variant_type::device> {
        static void convert(VARIANT& dst, const variant& src) {
            const auto device = src.get<variant_type::device>();
            const auto value = from_utf8(device->name());
            dst.vt = VT_BSTR;
            dst.bstrVal = ::SysAllocString(value.c_str());
        }
    };

    template<> struct excellent_converter<variant_type::environment> {
        static void convert(VARIANT& dst, const variant& src) {
            const auto env = src.get<variant_type::environment>();
            const auto value = from_utf8(env->name());
            dst.vt = VT_BSTR;
            dst.bstrVal = ::SysAllocString(value.c_str());
        }
    };


    /// <summary>
    /// Tries a <see cref="excellent_converter" /> on all given 
    /// <see cref="varient_type" />.
    /// </summary>
    template<variant_type T, variant_type... Ts>
    void convert_variant(detail::variant_type_list_t<T, Ts...>,
            VARIANT& dst,
            const variant& src) {
        if (src.type() == T) {
            excellent_converter<T>::convert(dst, src);
        } else {
            convert_variant(detail::variant_type_list_t<Ts...>(), dst, src);
        }
    }

    /// <summary>
    /// Recursion stop for <see cref="convert" />.
    /// </summary>
    inline void convert_variant(detail::variant_type_list_t<>, VARIANT& dst,
            const variant&) {
        ::VariantClear(&dst);
    }

} /* namespace trrojan */

#endif /* defined(_WIN32) && !defined(_UWP) */
