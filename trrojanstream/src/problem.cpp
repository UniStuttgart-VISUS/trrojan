/// <copyright file="problem.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/problem.h"


/*
 * trrojan::stream::problem::problem
 */
trrojan::stream::problem::problem(const trrojan::stream::scalar_type scalar,
        const size_t parallelism, const size_t size)
        : _parallelism(parallelism), _scalar_type(scalar) {
    switch (this->_scalar_type) {
        case trrojan::stream::scalar_type::float32:
            this->allocate<trrojan::stream::scalar_type::float32>(size);
            break;

        case trrojan::stream::scalar_type::float64:
            this->allocate<trrojan::stream::scalar_type::float64>(size);
            break;

        case trrojan::stream::scalar_type::int32:
            this->allocate<trrojan::stream::scalar_type::int32>(size);
            break;

        case trrojan::stream::scalar_type::int64:
            this->allocate<trrojan::stream::scalar_type::int64>(size);
            break;
    }
}
