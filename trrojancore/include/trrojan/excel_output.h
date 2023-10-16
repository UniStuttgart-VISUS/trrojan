// <copyright file="excel_output.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#if (defined(_WIN32) && !defined(TRROJAN_FOR_UWP))
#include <atlbase.h>
#include <Ole2.h>
#endif /* (defined(_WIN32) && !defined(TRROJAN_FOR_UWP)) */

#include "trrojan/excel_output_params.h"
#include "trrojan/output.h"


namespace trrojan {

#if (defined(_WIN32) && !defined(TRROJAN_FOR_UWP))
    /// <summary>
    /// Base class for output handlers.
    /// </summary>
    class TRROJANCORE_API excel_output : public output_base {

    public:

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
        excel_output(void);

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        virtual ~excel_output(void);

        /// <inheritdoc />
        virtual void close(void);

        /// <inheritdoc />
        virtual void open(const output_params& params);

        /// <inheritdoc />
        virtual output_base& operator <<(const basic_result& result);

    private:

        /// <summary>
        /// Get column name for zero-based (!) index 'col'.
        /// <summary>
        static std::wstring column_name(const long col);

        /// <summary>
        /// Convert to Excel-compatible string.
        /// </summary>
        static std::wstring convert(const std::string& str);

        /// <summary>
        /// Invoke the specified verb on <paramref name="dispatch" />.
        /// </summary>
        template<class... P>
        static void invoke(VARIANT *outResult, const int automationType,
            IDispatch *dispatch, LPOLESTR name, P&&... params);

        /// <summary>
        /// Get zero-based (!) cell range.
        /// <summary>
        ATL::CComPtr<IDispatch> get_range(const long firstRow,
            const long firstCol,const long lastRow, const long lastCol);

        /// <summary>
        /// Get zero-based (!) cell range in a single row.
        /// <summary>
        inline ATL::CComPtr<IDispatch> get_range(const long row,
                const long firstCol, const long lastCol) {
            return this->get_range(row, firstCol, row, lastCol);
        }

        /// <summary>
        /// Get zero-based (!) cell.
        /// <summary>
        inline ATL::CComPtr<IDispatch> get_range(const long row,
                const long col) {
            return this->get_range(row, col, col);
        }

        /// <summary>
        /// Gets the used range in <see cref="trrojan::excel_output::sheet" />.
        /// </summary>
        ATL::CComPtr<IDispatch> get_used_range(void);

        /// <summary>
        /// Gets the zero-based (!) last row in the active sheet.
        /// </summary>
        long last_row(void);

        /// <summary>
        /// Reads the value from the specified cell.
        /// </summary>
        void read_value(VARIANT& outValue, const long row, const long col);

        /// <summary>
        /// Save the workbook (at the specified path, if
        /// <paramref name="path" /> is not empty).
        /// </summary>
        void save(const std::string& path);

        /// <summary>
        /// Writes a formula to the specified cell.
        /// </summary>
        void write_formula(const std::wstring formula, const long row,
            const long col);

        /// <summary>
        /// Writes a value to the specified cell.
        /// </summary>
        void write_value(const variant& value, const long row,
            const long col);

        /// <summary>
        /// Writes a value to the specified cell.
        /// </summary>
        void write_value(const std::string& value, const long row,
            const long col);

        ATL::CComPtr<IDispatch> book;
        ATL::CComPtr<IDispatch> books;
        ATL::CComPtr<IDispatch> excel;
        ATL::CComPtr<IDispatch> sheet;
    };
#endif /* (defined(_WIN32) && !defined(TRROJAN_FOR_UWP)) */
}

#include "trrojan/excel_output.inl"
