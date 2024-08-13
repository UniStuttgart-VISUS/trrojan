// <copyright file="export.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once


#if (defined(_MSC_VER) && !defined(TRROJAND3D12_STATIC))

#ifdef TRROJAND3D12_EXPORTS
#define TRROJAND3D12_API __declspec(dllexport)
#else /* TRROJAND3D12_EXPORTS */
#define TRROJAND3D12_API __declspec(dllimport)
#endif /* TRROJAND3D12_EXPORTS*/

#else /* (defined(_MSC_VER) && !defined(TRROJAND3D12_STATIC)) */

#define TRROJAND3D12_API

#endif /* (defined(_MSC_VER) && !defined(TRROJAND3D12_STATIC)) */
