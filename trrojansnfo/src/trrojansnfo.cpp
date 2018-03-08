/// <copyright file="trrojansnfo.cpp" company="SFB-TRR 161 Quantitative Methods for Visual Computing">
/// Copyright © 2018 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
/// Copyright © 2018 SFB-TRR 161. Alle Rechte vorbehalten.
/// </copyright>
/// <author>Christoph Müller</author>


#if (defined(WIN32) && !defined(TRROJANSNFO_STATIC))
#include <Windows.h>


/// <summary>
/// Entry point of the DLL.
/// </summary>
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            ::DisableThreadLibraryCalls(hDll);
            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

#endif /* (defined(WIN32) && !defined(TRROJANSNFO_STATIC)) */
