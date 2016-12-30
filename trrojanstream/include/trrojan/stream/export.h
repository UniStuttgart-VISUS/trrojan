/// <copyright file="export.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2016 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJANSTREAM_STATIC))

#ifdef TRROJANSTREAM_EXPORTS
#define TRROJANSTREAM_API __declspec(dllexport)
#else /* TRROJANSTREAM_EXPORTS */
#define TRROJANSTREAM_API __declspec(dllimport)
#endif /* TRROJANSTREAM_EXPORTS*/

#else /* (defined(_MSC_VER) && !defined(TRROJANSTREAM_STATIC)) */

#define TRROJANSTREAM_API

#endif /* (defined(_MSC_VER) && !defined(TRROJANSTREAM_STATIC)) */
