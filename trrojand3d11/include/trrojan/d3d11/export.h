/// <copyright file="export.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
