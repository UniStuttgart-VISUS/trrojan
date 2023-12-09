// <copyright file="csv_output.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/csv_output.h"

#include <sstream>
#include <stdexcept>

#include "trrojan/csv_util.h"



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

    // Note: We use the binary mode such that we can control the type of line
    // break being generated.
    this->file.open(this->params->path(), std::ios::trunc | std::ios::binary);
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
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << sep;
            }
            this->print(c.name());
        }

        for (auto& n : result.result_names()) {
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << sep;
            }
            this->print(n);
        }

        this->file << nl;
        this->first_line = false;
    }

    for (size_t i = 0; i < result.measurements(); ++i) {
        isFirst = true;
        for (auto& c : result.configuration()) {
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << sep;
            }
            this->print(c.value());
        }

        for (size_t j = 0; j < result.values_per_measurement(); ++j) {
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << sep;
            }
            this->print(result.raw_result(i, j));
        }

        this->file << nl;
    }

    this->file.flush();

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
    print_csv_value(this->file, v, this->params->quote_string());
}
