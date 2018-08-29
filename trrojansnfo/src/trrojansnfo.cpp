/// <copyright file="trrojansnfo.cpp" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>


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
