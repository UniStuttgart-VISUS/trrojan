// <copyright file="unique_variable.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2021 - 2023 Visualisierungsinstitut der Universität Stuttgart.
// </copyright>
// <author>Christoph Müller</author>


#define _TRROJAN_CONCAT0(l, r) l##r
#define _TRROJAN_CONCAT(l, r) _TRROJAN_CONCAT0(l, r)

// Andrei Alexandrescu's unique variable generator.
#if defined(__COUNTER__)
#define _TRROJAN_UNIQUE_VARIABLE(p) _TRROJAN_CONCAT(p, __COUNTER__)
#else /* defined(__COUNTER__) */
#define _TRROJAN_UNIQUE_VARIABLE(p) _TRROJAN_CONCAT(p, __LINE__)
#endif /* defined(__COUNTER__) */
