/// <copyright file="export.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJANCL_STATIC))

#ifdef TRROJANCL_EXPORTS
#define TRROJANCL_API __declspec(dllexport)
#else /* TRROJANCL_EXPORTS */
#define TRROJANCL_API __declspec(dllimport)
#endif /* TRROJANCL_EXPORTS*/

#else /* (defined(_MSC_VER) && !defined(TRROJANCL_STATIC)) */

#define TRROJANCL_API

#endif /* (defined(_MSC_VER) && !defined(TRROJANCL_STATIC)) */
