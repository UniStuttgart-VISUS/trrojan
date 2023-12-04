// <copyright file="r_output.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#include "trrojan/r_output.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>


/*
 * trrojan::r_output::r_output
 */
trrojan::r_output::r_output(void) : line(0) { }


/*
 * trrojan::r_output::~r_output
 */
trrojan::r_output::~r_output(void) {
    this->close();
}


/*
 * trrojan::r_output::close
 */
void trrojan::r_output::close(void) {
    if (this->file.is_open()) {
        this->file << this->conversion.str();
        this->file.close();
    }
}


/*
 * trrojan::r_output::open
 */
void trrojan::r_output::open(const output_params& params) {
    if (params == nullptr) {
        throw std::invalid_argument("'params' must not be nullptr.");
    }

    this->params = std::dynamic_pointer_cast<params_type>(params);
    if (this->params == nullptr) {
        this->params = std::make_shared<params_type>(*params);
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

    this->line = 0;
}


/*
 * trrojan::r_output::operator <<
 */
trrojan::output_base& trrojan::r_output::operator <<(
        const basic_result& result) {
    if ((this->params == nullptr) || !this->file) {
        throw std::logic_error("The output must be opened before data can be "
            "written.");
    }

    auto nl = this->params->line_break();
    auto rows = (std::max)(static_cast<size_t>(1), this->params->expected_rows());
    auto& var = this->params->variable_name();

    if (this->line == 0) {
        auto cnt = std::string("cnt_") + var;

        // Fill the post-processing suffix for after the data.
        this->conversion.clear();
        this->conversion << var << " <- na.omit(" << var << ")" << nl;

        // Declare a variable with the expected number of rows.
        this->file << cnt << " <- " << rows << nl;

        // Initialise the data frame with NAs.
        // https://stackoverflow.com/questions/3642535/creating-an-r-dataframe-row-by-row
        this->file << var << " <- data.frame(";

        for (auto& c : result.configuration()) {
            this->file << c.name() << " = rep(NA, " << cnt << "), ";
            this->conversion << var << "$" << c.name() << " <- as.factor("
                << var << "$" << c.name() << ")" << nl;
        }

        for (auto& n : result.result_names()) {
            this->file << n << " = rep(NA, " << cnt << "), ";
        }

        this->file << "stringsAsFactors = TRUE)" << nl;

        // Create a list of all factor names.
        {
            this->file << var << "_factors <- c(";
            auto isFirst = true;
            for (auto& c : result.configuration()) {
                if (isFirst) {
                    isFirst = false;
                } else {
                    this->file << ", ";
                }
                this->print(c.name());
            }
            this->file << ")" << nl;
        }
    }

    for (size_t i = 0; i < result.measurements(); ++i) {
        ++this->line;
        auto isFirst = true;

        this->file << var << "[" << this->line << ",] <- list(";

        for (auto& c : result.configuration()) {
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << ", ";
            }
            this->print_factor(c);
        }

        for (size_t j = 0; j < result.values_per_measurement(); ++j) {
            if (isFirst) {
                isFirst = false;
            } else {
                this->file << ", ";
            }
            this->print(result.raw_result(i, j));
        }

        this->file << ")" << nl;
    }

    return *this;
}


/*
 * trrojan::r_output::print
 */
void trrojan::r_output::print(const std::string& str) {
    this->file << "\"" << str << "\"";
}


/*
 * trrojan::r_output::print
 */
void trrojan::r_output::print(const variant& v) {
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

    if (isString) {
        this->file << "\"" << v << "\"";
    } else {
        this->file << v;
    }
}


/*
 * trrojan::r_output::print_factor
 */
void trrojan::r_output::print_factor(const named_variant& v) {
    std::stringstream s;
    s << v.value();

    // Add quotes to make it a factor.
    this->file << "\"";
    for (auto c : s.str()) {
        switch (c) {
            // Make sure that we output escaped string for R.
            case '\\': this->file << "\\\\"; break;
            default: this->file << c; break;
        }
    }
    this->file << "\"";
}
