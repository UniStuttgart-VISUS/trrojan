/// <copyright file="power_state_scope.h" company="Visualisierungsinstitut der Universit�t Stuttgart">
/// Copyright � 2016 - 2018 Visualisierungsinstitut der Universit�t Stuttgart. Alle Rechte vorbehalten.
/// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
/// </copyright>
/// <author>Christoph M�ller</author>

#pragma once

#if defined(_WIN32)
#include <winsdkver.h>

#if (defined(_WIN32_MAXVER) && (_WIN32_MAXVER >= 0x0A00))
#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>

#define TRROJAN_WITH_POWER_STATE_SCOPE (1)
#pragma comment(lib, "d3d12.lib")
#endif /* (defined(_WIN32_MAXVER) && (_WIN32_MAXVER >= 0x0A00)) */

#include <atlbase.h>
#endif /* defined _WIN32 */

#include <vector>

#include "trrojan/export.h"


namespace trrojan {

    /// <summary>
    /// This class enumerates all GPUs using Direct3D 12 and enables their
    /// stable power state (in other words: disables GPU boost) while the
    /// respective object lives.
    /// </summary>
    class TRROJANCORE_API power_state_scope {

    public:

        /// <summary>
        /// Initialises the instance by creating a D3D12 device for all
        /// adapters and enabling their stable power state.
        /// </summary>
        power_state_scope(void);

        /// <summary>
        /// Finalises the instance by deleting all devices forcing the graphics
        /// adapters into a stable power state.
        /// </summary>
        ~power_state_scope(void);

    private:

#if defined(TRROJAN_WITH_POWER_STATE_SCOPE)
        /// <summary>
        /// Stores the devices that are used to enforce the stable power state.
        /// </summary>
        std::vector<ATL::CComPtr<ID3D12Device>> devices;
#endif /* defined(TRROJAN_WITH_POWER_STATE_SCOPE) */

    };
}
