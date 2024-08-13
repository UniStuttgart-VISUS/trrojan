/// <copyright file="export.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

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
