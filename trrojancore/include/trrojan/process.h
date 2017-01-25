/// <copyright file="process.h" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2017 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>

#pragma once

#include <string>

#ifdef _WIN32
#include <Windows.h>
#else /* _WIN32 */
#include <unistd.h>
#endif /* _WIN32 */

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// The native type of a process ID.
    /// </summary>
#ifdef _WIN32
    typedef DWORD process_id;
#else /* _WIN32 */
    typedef pid_t process_id;
#endif /* _WIN32 */

    /// <summary>
    /// Answer the file name of the executable fo the currently executing
    /// process.
    /// </summary>
    std::string TRROJANCORE_API get_module_file_name(void);

    /// <summary>
    /// Answer the ID of the calling process.
    /// </summary>
    process_id TRROJANCORE_API get_process_id(void);
}
