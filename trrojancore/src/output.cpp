/// <copyright file="output.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 - 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/output.h"


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
