/// <copyright file="console_output.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/console_output.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>



/*
 * trrojan::console_output::console_output
 */
trrojan::console_output::console_output(void) : padding(0) { }


/*
 * trrojan::console_output::~console_output
 */
trrojan::console_output::~console_output(void) { }


/*
 * trrojan::console_output::close
 */
void trrojan::console_output::close(void) { }


/*
 * trrojan::console_output::open
 */
void trrojan::console_output::open(const output_params& params) {
    if (params == nullptr) {
        throw std::invalid_argument("'params' must not be nullptr.");
    }

    this->params = std::dynamic_pointer_cast<console_output_params>(params);
    if (this->params == nullptr) {
        this->params = std::make_shared<console_output_params>(*params);
    }
}


/*
 * trrojan::console_output::operator <<
 */
trrojan::output_base& trrojan::console_output::operator <<(
        const basic_result& result) {
    if (this->params == nullptr) {
        throw std::logic_error("The output must be opened before data can be "
            "written.");
    }

    if (this->padding == 0) {
        typedef std::decay<decltype(result.configuration())>::type::value_type
            conf_type;
        typedef std::decay<decltype(result.result_names())>::type::value_type
            res_type;

        auto itConf = std::max_element(
            result.configuration().begin(),
            result.configuration().end(),
            [](const conf_type& l, const conf_type& r)
            { return l.name().length() < r.name().length(); });
        auto itRes = std::max_element(
            result.result_names().begin(),
            result.result_names().end(),
            [](const res_type& l, const res_type& r)
            { return l.length() < r.length(); });

        auto maxConf = (itConf != result.configuration().end())
            ? itConf->name().length() : 0;
        auto maxRes = (itRes != result.result_names().end())
            ? itRes->length() : 0;

        this->padding = (std::max)(maxConf, maxRes) + 1;
    }

    for (size_t i = 0; i < result.measurements(); ++i) {
        for (auto& c : result.configuration()) {
            this->print_name(c.name());
            std::cout << c.value() << std::endl;
        }

        for (size_t j = 0; j < result.values_per_measurement(); ++j) {
            this->print_name(result.result_names()[j]);
            std::cout << result.raw_result(i, j) << std::endl;
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;

    return *this;
}


/*
 * trrojan::console_output::print_name
 */
void trrojan::console_output::print_name(const std::string& str) {
    const auto len = str.length();

    std::cout << str;

    for (size_t i = len; i < this->padding; ++i) {
        std::cout << ' ';
    }

    std::cout << ": ";
}
