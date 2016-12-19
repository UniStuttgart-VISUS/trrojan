/// <copyright file="export.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJANCORE_STATIC))

#ifdef TRROJANCORE_EXPORTS
#define TRROJANCORE_API __declspec(dllexport)
#else /* TRROJANCORE_EXPORTS */
#define TRROJANCORE_API __declspec(dllimport)
#endif /* TRROJANCORE_EXPORTS*/

#else /* (defined(_MSC_VER) && defined(TRROJANCORE_DLL)) */

#define TRROJANCORE_API

#endif /*  (defined(_MSC_VER) && !defined(TRROJANCORE_STATIC)) */
