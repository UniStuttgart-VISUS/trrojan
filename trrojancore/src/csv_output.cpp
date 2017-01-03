/// <copyright file="csv_output.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/csv_output.h"

#include <sstream>
#include <stdexcept>



/*
 * trrojan::csv_output::csv_output
 */
trrojan::csv_output::csv_output(void) : first_line(false) { }


/*
 * trrojan::csv_output::~csv_output
 */
trrojan::csv_output::~csv_output(void) {
    this->close();
}


/*
 * trrojan::csv_output::close
 */
void trrojan::csv_output::close(void) {
    if (this->file.is_open()) {
        this->file.close();
    }
}


/*
 * trrojan::csv_output::open
 */
void trrojan::csv_output::open(const output_params& params) {
    if (params == nullptr) {
        throw std::invalid_argument("'params' must not be nullptr.");
    }

    this->params = std::dynamic_pointer_cast<csv_output_params>(params);
    if (this->params == nullptr) {
        this->params = std::make_shared<csv_output_params>(*params);
    }

    this->file = std::move(std::ofstream(this->params->path(),
        std::ios::trunc));
    if (!this->file) {
        std::stringstream msg;
        msg << "Failed to open output file \"" << this->params->path() << "\""
            << std::ends;
        throw std::runtime_error(msg.str());
    }

    this->first_line = true;
}


/*
 * trrojan::csv_output::operator <<
 */
trrojan::output_base& trrojan::csv_output::operator <<(
        const basic_result& result) {
    if ((this->params == nullptr) || !this->file) {
        throw std::logic_error("The output must be opened before data can be "
            "written.");
    }

    auto isFirst = true;
    auto nl = this->params->line_break();
    auto sep = this->params->separator();

    if (this->first_line) {
        for (auto& c : result.configuration()) {
            if (!isFirst) {
                this->file << sep;
                isFirst = false;
            }
            this->print(c.name());
        }

        for (auto& n : result.result_names()) {
            if (!isFirst) {
                this->file << sep;
                isFirst = false;
            }
            this->print(n);
        }

        this->file << nl;
        this->first_line = false;
    }

    for (size_t i = 0; i < result.measurements(); ++i) {
        isFirst = true;
        for (auto& c : result.configuration()) {
            if (!isFirst) {
                this->file << sep;
                isFirst = false;
            }
            this->print(c.value());
        }

        for (size_t j = 0; j < result.values_per_measurement(); ++j) {
            if (!isFirst) {
                this->file << sep;
                isFirst = false;
            }
            this->print(result.raw_result(i, j));
        }

        this->file << nl;
    }

    return *this;
}


/*
 * trrojan::csv_output::print
 */
void trrojan::csv_output::print(const std::string& str) {
    if (this->params->quote_string()) {
        this->file << "\"" << str << "\"";
    } else {
        this->file << str;
    }
}


/*
 * trrojan::csv_output::print
 */
void trrojan::csv_output::print(const variant& v) {
    bool isString;
    switch (v.type()) {
        case variant_type::string:
        case variant_type::wstring:
        case variant_type::device:
        case variant_type::environment:
            isString = true;
            break;

        default:
            isString = false;
    }

    if (this->params->quote_string() && isString) {
        this->file << "\"" << v << "\"";
    } else {
        this->file << v;
    }
}
