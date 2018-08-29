/// <copyright file="export.h" company="Visualisierungsinstitut der Universität Stuttgart">
/// Copyright © 2016 - 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
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
