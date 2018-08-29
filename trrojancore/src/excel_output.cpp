/// <copyright file="excel_output.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/excel_output.h"


#ifdef _WIN32
#include <codecvt>
#include <locale>
#include <memory>

#include "trrojan/log.h"


/*
 * trrojan::excel_output::excel_output
 */
trrojan::excel_output::excel_output(void) { }


/*
 * trrojan::excel_output::~excel_output
 */
trrojan::excel_output::~excel_output(void) {
    try {
        this->close();
    } catch (...) { /* Ignore this. */ }
}


/*
 * trrojan::excel_output::close
 */
void trrojan::excel_output::close(void) {
    if (this->excel != nullptr) {
        //// Set .Saved property of workbook to TRUE so we aren't prompted
        //// to save when we tell Excel to quit...
        //{
        //    VARIANT x;
        //    x.vt = VT_I4;
        //    x.lVal = 1;
        //    AutoWrap(DISPATCH_PROPERTYPUT, NULL, pXlBook, L"Saved", 1, x);
        //}

        excel_output::invoke(NULL, DISPATCH_METHOD, this->excel, L"Quit");

        this->sheet = nullptr;
        this->book = nullptr;
        this->books = nullptr;
        this->excel = nullptr;

        ::CoUninitialize();
    }
}


/*
 * trrojan::excel_output::open
 */
void trrojan::excel_output::open(const output_params& params) {
    CLSID clsid;                // CLSID of Excel application.

    /* Make sure that any previous instance of Excel is closed. */
    this->close();

    /* Initialise COM. */
    // Note: DXGI uses STA, so we need to do so, too. auto hr = ::CoInitializeEx(0, COINIT_MULTITHREADED);
    auto hr = ::CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to initialise COM to communicate with Excel: " << hr
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Get CLSID of Excel. */
    hr = ::CLSIDFromProgID(L"Excel.Application", &clsid);
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to obtain CLSID of Excel: " << hr << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Start Excel server. */
    assert(this->excel == nullptr);
    hr = ::CoCreateInstance(clsid, nullptr, CLSCTX_LOCAL_SERVER, IID_IDispatch,
        reinterpret_cast<void **>(&this->excel));
    if (FAILED(hr)) {
        std::stringstream msg;
        msg << "Failed to instantiate Excel server: " << hr << std::ends;
        throw std::runtime_error(msg.str());
    }

    /* Obtain customised parameters for Excel. */
    auto p = std::dynamic_pointer_cast<excel_output_params>(params);
    if (p == nullptr) {
        p = std::make_shared<excel_output_params>(*params);
    }

    /* Make Excel visible if requested. */
    if (p->is_visible()) {
        VARIANT input;
        input.vt = VT_I4;
        input.lVal = 1;
        excel_output::invoke(nullptr, DISPATCH_PROPERTYPUT, this->excel,
            L"Visible", input);
    }

    /* Get Workbooks collection. */
    {
        VARIANT output;
        ::VariantInit(&output);
        excel_output::invoke(&output, DISPATCH_PROPERTYGET, this->excel,
            L"Workbooks");
        this->books = output.pdispVal;
    }

    /* Add a new workbook for the results. */
    {
        VARIANT output;
        ::VariantInit(&output);
        excel_output::invoke(&output, DISPATCH_PROPERTYGET, this->books,
            L"Add");
        this->book = output.pdispVal;
    }

    /* Get the active sheet. */
    {
        VARIANT output;
        ::VariantInit(&output);
        excel_output::invoke(&output, DISPATCH_PROPERTYGET, this->excel,
            L"ActiveSheet");
        this->sheet = output.pdispVal;
    }

    /* Save the workbook to the specified file. */
    this->save(p->path());

    log::instance().write_line(log_level::verbose, "Excellent choice!");
}


/*
 * trrojan::excel_output::operator <<
 */
trrojan::output_base& trrojan::excel_output::operator <<(
        const basic_result& result) {
    if (this->excel == nullptr) {
        throw std::logic_error("The output must be opened before data can be "
            "written.");
    }

    static const std::string emptyString;
    long row = this->last_row();
    long col = 0;

    if (row == 0) {
        for (auto& c : result.configuration()) {
            this->write_value(c.name(), row, col++);
        }

        for (auto& n : result.result_names()) {
            this->write_value(n, row, col++);
        }
        ++row;
    }

    for (size_t i = 0; i < result.measurements(); ++i, ++row) {
        col = 0;
        for (auto& c : result.configuration()) {
            this->write_value(c.value(), row, col++);
        }

        for (size_t j = 0; j < result.values_per_measurement(); ++j) {
            this->write_value(result.raw_result(i, j), row, col++);
        }
    }

    this->save(emptyString);

    return *this;
}


/*
 * trrojan::excel_output::column_name
 */
std::wstring trrojan::excel_output::column_name(const long col) {
    const long RANGE = L'Z' - L'A' + 1;
    long d = col + 1;
    long r = 0;
    std::wstring retval;

    while (d > 0) {
        r = (d - 1) % RANGE;
        retval = std::wstring(1, (L'A' + static_cast<wchar_t>(r))) + retval;
        d = (d - r) / RANGE;
    }

    return retval;
}


/*
 * trrojan::excel_output::convert
 */
std::wstring trrojan::excel_output::convert(const std::string & str) {
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
    return cvt.from_bytes(str);
}


/*
 * trrojan::excel_output::get_range
 */
ATL::CComPtr<IDispatch> trrojan::excel_output::get_range(const long firstRow,
        const long firstCol, const long lastRow, const long lastCol) {
    VARIANT input;                  // Input of automation call.
    VARIANT output;                 // Output of automation call.
    ATL::CComPtr<IDispatch> range;  // Range of new row.
    std::wstringstream rangeSpec;   // String representation of the range.

    /* Prepare input. */
    VariantInit(&input);
    rangeSpec << excel_output::column_name(firstCol) << (firstRow + 1)
        << L":" << excel_output::column_name(lastCol) << (lastRow + 1)
        << std::ends;
    input.vt = VT_BSTR;
    input.bstrVal = ::SysAllocString(rangeSpec.str().c_str());

    /* Prepare output. */
    VariantInit(&output);

    /* Make call. */
    assert(this->sheet != nullptr);
    try {
        excel_output::invoke(&output, DISPATCH_PROPERTYGET, this->sheet,
            L"Range", input);
        ::VariantClear(&input);
        range = output.pdispVal;
    } catch (...) {
        ::VariantClear(&input);
        throw;
    }

    return range;
}


/*
 * trrojan::excel_output::get_used_range
 */
ATL::CComPtr<IDispatch> trrojan::excel_output::get_used_range(void) {
    VARIANT output;                 // Output of automation call.

    /* Prepare output. */
    VariantInit(&output);

    /* Make call. */
    assert(this->sheet != nullptr);
    excel_output::invoke(&output, DISPATCH_PROPERTYGET, this->sheet,
        L"UsedRange");

    return output.pdispVal;
}


/*
 * trrojan::excel_output::last_row
 */
long trrojan::excel_output::last_row(void) {
    VARIANT output;                 // Output of automation call.

    /* Get the used range. */
    auto range = this->get_used_range();

    /* Retrieve the used rows. */
    ::VariantInit(&output);
    excel_output::invoke(&output, DISPATCH_PROPERTYGET, range, L"Rows");
    range = output.pdispVal;
    ::VariantClear(&output);

    /* Retrieve number of rows. */
    ::VariantInit(&output);
    excel_output::invoke(&output, DISPATCH_PROPERTYGET, range, L"Count");
    auto retval = output.lVal - 1;
    ::VariantClear(&output);

    return retval;
}


/*
 * trrojan::excel_output::read_value
 */
void trrojan::excel_output::read_value(VARIANT& outValue,
        const long row, const long col) {
    /* Get the range to read from. */
    auto range = this->get_range(row, col);
    assert(range != nullptr);

    /* Get the value. */
    ::VariantInit(&outValue);
    excel_output::invoke(&outValue, DISPATCH_PROPERTYGET, range, L"Value");
}


/*
 * trrojan::excel_output::save
 */
void trrojan::excel_output::save(const std::string& path) {
    if (path.empty()) {
        excel_output::invoke(nullptr, DISPATCH_METHOD, this->book, L"Save");

    } else {
        VARIANT input;
        ::VariantInit(&input);
        input.vt = VT_BSTR;
        input.bstrVal = ::SysAllocString(excel_output::convert(path).c_str());
        try {
            excel_output::invoke(nullptr, DISPATCH_METHOD, this->book,
                L"SaveAs", input);
            ::VariantClear(&input);
        } catch (...) {
            ::VariantClear(&input);
            throw;
        }
    }
}


/*
 * trrojan::excel_output::write_formula
 */
void trrojan::excel_output::write_formula(const std::wstring formula,
        const long row, const long col) {
    VARIANT inputCell;

    /* Prepare input. */
    ::VariantInit(&inputCell);
    inputCell.vt = VT_BSTR;
    inputCell.bstrVal = ::SysAllocString(formula.c_str());

    /* Get the range we write to. */
    auto range = this->get_range(row, col);
    assert(range != nullptr);

    /* Set range to 'inputRow'. */
    try {
        excel_output::invoke(nullptr, DISPATCH_PROPERTYPUT, range, L"Formula",
            inputCell);
        ::VariantClear(&inputCell);
    } catch (...) {
        ::VariantClear(&inputCell);
        throw;
    }
}


/*
 * trrojan::excel_output::write_value
 */
void trrojan::excel_output::write_value(const variant& value, const long row,
        const long col) {
    VARIANT inputCell;
    std::stringstream ss;

    /* Prepare input. */
    ::VariantInit(&inputCell);
    // TODO: this is suboptimal. Find a way to template VARIANT, too.
    switch (value.type()) {
        case variant_type::boolean:
            inputCell.vt = VT_BOOL;
            inputCell.boolVal = value.get<variant_type::boolean>();
            break;

        case variant_type::int8:
            inputCell.vt = VT_I1;
            inputCell.cVal = value.get<variant_type::int8>();
            break;

        case variant_type::int16:
            inputCell.vt = VT_I2;
            inputCell.iVal = value.get<variant_type::int16>();
            break;

        case variant_type::int32:
            inputCell.vt = VT_I4;
            inputCell.intVal = value.get<variant_type::int32>();
            break;

        case variant_type::int64:
            inputCell.vt = VT_I8;
            inputCell.llVal = value.get<variant_type::int64>();
            break;

        case variant_type::uint8:
            inputCell.vt = VT_UI1;
            inputCell.bVal = value.get<variant_type::uint8>();
            break;

        case variant_type::uint16:
            inputCell.vt = VT_UI2;
            inputCell.uiVal = value.get<variant_type::uint16>();
            break;

        case variant_type::uint32:
            inputCell.vt = VT_UI4;
            inputCell.uintVal = value.get<variant_type::uint32>();
            break;

        case variant_type::uint64:
            inputCell.vt = VT_UI8;
            inputCell.ullVal = value.get<variant_type::uint64>();
            break;

        case variant_type::float32:
            inputCell.vt = VT_R4;
            inputCell.fltVal = value.get<variant_type::float32>();
            break;

        case variant_type::float64:
            inputCell.vt = VT_R8;
            inputCell.dblVal = value.get<variant_type::float64>();
            break;

        case variant_type::string:
            inputCell.vt = VT_BSTR;
            inputCell.bstrVal = ::SysAllocString(excel_output::convert(
                value.get<variant_type::string>()).c_str());
            break;

        case variant_type::wstring:
            inputCell.vt = VT_BSTR;
            inputCell.bstrVal = ::SysAllocString(
                value.get<variant_type::wstring>().c_str());
            break;

        case variant_type::device:
            inputCell.vt = VT_BSTR;
            ss.clear();
            ss << value.get<variant_type::device>() << std::ends;
            inputCell.bstrVal = ::SysAllocString(excel_output::convert(
                ss.str()).c_str());
            break;

        case variant_type::environment:
            inputCell.vt = VT_BSTR;
            ss.clear();
            ss << value.get<variant_type::environment>() << std::ends;
            inputCell.bstrVal = ::SysAllocString(excel_output::convert(
                ss.str()).c_str());
            break;
    }

    /* Get the range we write to. */
    auto range = this->get_range(row, col);
    assert(range != nullptr);

    /* Set range to 'inputRow'. */
    try {
        excel_output::invoke(nullptr, DISPATCH_PROPERTYPUT, range, L"Value",
            inputCell);
        ::VariantClear(&inputCell);
    } catch (...) {
        ::VariantClear(&inputCell);
        throw;
    }
}


/*
 * trrojan::excel_output::write_value
 */
void trrojan::excel_output::write_value(const std::string& value,
        const long row, const long col) {
    VARIANT inputCell;

    /* Prepare input. */
    ::VariantInit(&inputCell);
    inputCell.vt = VT_BSTR;
    inputCell.bstrVal = ::SysAllocString(excel_output::convert(value).c_str());

    /* Get the range we write to. */
    auto range = this->get_range(row, col);
    assert(range != nullptr);

    /* Set range to 'inputRow'. */
    try {
        excel_output::invoke(nullptr, DISPATCH_PROPERTYPUT, range, L"Value",
            inputCell);
        ::VariantClear(&inputCell);
    } catch (...) {
        ::VariantClear(&inputCell);
        throw;
    }
}
#endif /* _WIN32 */
