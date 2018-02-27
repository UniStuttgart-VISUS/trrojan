/// <copyright file="export.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJAND3D11_STATIC))

#ifdef TRROJAND3D11_EXPORTS
#define TRROJAND3D11_API __declspec(dllexport)
#else /* TRROJAND3D11_EXPORTS */
#define TRROJAND3D11_API __declspec(dllimport)
#endif /* TRROJAND3D11_EXPORTS*/

#else /* (defined(_MSC_VER) && !defined(TRROJAND3D11_STATIC)) */

#define TRROJAND3D11_API

#endif /* (defined(_MSC_VER) && !defined(TRROJAND3D11_STATIC)) */
