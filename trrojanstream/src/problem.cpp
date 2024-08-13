/// <copyright file="problem.cpp" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#include "trrojan/stream/problem.h"


/*
 * trrojan::stream::problem::problem
 */
trrojan::stream::problem::problem(const scalar_type_t scalar,
        const trrojan::variant& value,
        const task_type_t task,
        const access_pattern_t pattern,
        const size_t size,
        const size_t iterations,
        const size_t parallelism)
        : _access_pattern(pattern),
        _iterations(iterations),
        _parallelism(parallelism),
        _scalar_size(0),
        _scalar_type(scalar),
        _scalar_value(value),
        _task_type(task) {
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
