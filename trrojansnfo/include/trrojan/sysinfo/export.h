/// <copyright file="export.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJANSNFO_STATIC))

#ifdef TRROJANSNFO_EXPORTS
#define TRROJANSNFO_API __declspec(dllexport)
#else /* TRROJANSNFO_EXPORTS */
#define TRROJANSNFO_API __declspec(dllimport)
#endif /* TRROJANSNFO_EXPORTS*/

#else /* (defined(_MSC_VER) && !defined(TRROJANSNFO_STATIC)) */

#define TRROJANSNFO_API

#endif /* (defined(_MSC_VER) && !defined(TRROJANSNFO_STATIC)) */
