/// <copyright file="export.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

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
