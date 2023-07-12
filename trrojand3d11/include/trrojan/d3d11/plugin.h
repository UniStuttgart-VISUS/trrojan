// <copyright file="plugin.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2022 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include "trrojan/plugin.h"

#include <winrt/windows.ui.core.h>

#include <cinttypes>
#include <string>
#include <vector>

#include <Windows.h>

#include "trrojan/d3d11/export.h"


namespace trrojan {
namespace d3d11 {

    /// <summary>
    /// Descriptor for the Direct3D plugin.
    /// </summary>
    class TRROJAND3D11_API plugin : public trrojan::plugin_base {

    public:

        typedef trrojan::plugin_base::benchmark_list benchmark_list;
        typedef trrojan::plugin_base::environment_list environment_list;

        /// <summary>
        /// Retrieves the directory where the plugin is located.
        /// </summary>
        /// <returns>The location of the plugin.</returns>
        static std::string get_directory(void);

        /// <summary>
        /// Gets the location of the plugin.
        /// </summary>
        /// <returns>The path to the plugin DLL.</returns>
        static std::string get_location(void);

        /// <summary>
        /// Load a resource from the Direct3D plugin's DLL.
        /// </summary>
        /// <param name="name">The name of the resource. Alternately, rather
        /// than a pointer, this parameter can be MAKEINTRESOURCE(ID), where ID
        /// is the integer identifier of the resource.</param>
        /// <param name="type">The resource type. Alternately, rather than a
        /// pointer, this parameter can be MAKEINTRESOURCE(ID), where ID is the
        /// integer identifier of the given resource type. </param>
        /// <returns>The content of the resource.</returns>
        static std::vector<std::uint8_t> load_resource(LPCTSTR name,
            LPCSTR type);

        inline plugin(
#ifdef _UWP
            winrt::Windows::UI::Core::CoreWindow const& window
#endif
        ) : trrojan::plugin_base("d3d11")
#ifdef _UWP
            , m_window(window) 
#endif
        { }

        virtual ~plugin(void);

        virtual size_t create_benchmarks(benchmark_list& dst) const;

        virtual size_t create_environments(environment_list& dst) const;

#ifdef _UWP
        // Cached reference to the Window.
        winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow> m_window{ nullptr };
#endif

    };

}
}
