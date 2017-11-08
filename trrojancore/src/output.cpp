/// <copyright file="output.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/output.h"

#include "trrojan/csv_output.h"
#include "trrojan/excel_output.h"
#include "trrojan/log.h"
#include "trrojan/text.h"


/*
 * trrojan::output_base::~output_base
 */
trrojan::output_base::~output_base(void) { }


/*
 * trrojan::output_base::operator <<
 */
trrojan::output_base& trrojan::output_base::operator <<(const result result) {
    return (result) ? (*this << *result) : *this;
}


/*
 * trrojan::output_base::operator <<
 */
trrojan::output_base& trrojan::output_base::operator <<(
        const result_set& results) {
    for (auto& r : results) {
        *this << r;
    }
    return *this;
}


/*
 * trrojan::make_output
 */
trrojan::output TRROJANCORE_API trrojan::make_output(const std::string& path) {
    auto idx = path.rfind('.');
    auto ext = tolower(path.substr(idx));

    if ((ext == ".csv") || (ext == ".txt")) {
        return std::make_shared<csv_output>();
    } else if (ext == ".xslx") {
        return std::make_shared<excel_output>();
    } else {
        log::instance().write_line(log_level::warning, "The file name "
            "extension \"%s\" of path \"%s\" cannot be use to determine "
            "the output type.", path);
        return nullptr;
    }
}
