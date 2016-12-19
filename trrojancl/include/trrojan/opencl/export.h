/// <copyright file="export.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
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
