// <copyright file="executive.h" company="Visualisierungsinstitut der Universität Stuttgart">
// Copyright © 2016 - 2023 Visualisierungsinstitut der Universität Stuttgart. Alle Rechte vorbehalten.
// Licensed under the MIT licence. See LICENCE.txt file in the project root for full licence information.
// </copyright>
// <author>Christoph Müller</author>

#pragma once

#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif /* _WIN32 */

#if defined(TRROJAN_FOR_UWP)
#include <winrt/windows.ui.core.h>
#include <winrt/windows.storage.h>
#endif /* defined(TRROJAN_FOR_UWP) */

#include "trrojan/cool_down.h"
#include "trrojan/environment.h"
#include "trrojan/export.h"
#include "trrojan/image_helper.h"
#include "trrojan/output.h"
#include "trrojan/power_collector.h"
#include "trrojan/plugin.h"
#include "trrojan/trroll_parser.h"


namespace trrojan {

    /// <summary>
    /// This is the root class which manages all the plugins and the
    /// environments.
    /// </summary>
    class TRROJANCORE_API executive {

    public:

        /// <summary>
        /// Defines the type used to pass a troll file to the executive.
        /// </summary>
        typedef trroll_parser::troll_input_type troll_input_type;

#if defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// The type used to reference the core window of an UWP application.
        /// </summary>
        typedef winrt::agile_ref<winrt::Windows::UI::Core::CoreWindow>
            window_type;
#endif /* defined(TRROJAN_FOR_UWP) */

#if defined(_WIN32)
        /// <summary>
        /// Gets the path of the directory where the exeuctable is located, but
        /// only on Windows.
        /// </summary>
        /// <returns>The directory containing the executable, which is
        /// guaranteed to end with <see cref="directory_separator_char" />.
        /// </returns>
        static std::string executable_directory(void);
#endif /* defined(_WIN32) */

#if defined(_WIN32)
        /// <summary>
        /// Gets the path to the TRRojan executable, but only on Windows.
        /// </summary>
        /// <returns>The path to the executable.</returns>
        static std::string executable_path(void);
#endif /* defined(_WIN32) */

        /// <summary>
        /// Creates a factor for the given <see cref="power_collector" />.
        /// </summary>
        /// <param name="c">The power collector to pass on as a factor. It is
        /// safe to pass <c>nullptr</c>.</param>
        /// <returns>A factor containing <paramref name="c" />.</returns>
        static inline factor make_factor(const power_collector::pointer& c) {
            return factor::from_manifestations(power_collector::factor_name, c);
        }

        /// <summary>
        /// The name of the factor passing the UWP core window in UWP builds.
        /// </summary>
        static const std::string factor_core_window;

        /// <summary>
        /// A string factor that allows for specifying where to look for the
        /// data sets, which might be useful when separating the benchmarking
        /// scripts from the data set storage. If not set, the application will
        /// usually assume that relative paths are relative to the working
        /// directory of TRRojan.
        /// </summary>
        static const std::string factor_data_folder;

        /// <summary>
        /// Initialises a new instance.
        /// </summary>
#if defined(TRROJAN_FOR_UWP)
        executive(window_type core_window);
#else /* defined(TRROJAN_FOR_UWP) */
        inline executive(void) = default;
#endif /* defined(TRROJAN_FOR_UWP) */

        executive(const executive&) = delete;

        /// <summary>
        /// Finalises the instance.
        /// </summary>
        ~executive(void);

        /// <summary>
        /// Search for the plugin with the specified name.
        /// </summary>
        /// <param name="name"></param>
        /// <returns>A pointer to the respective plugin or <c>nullptr</c> if the
        /// plugin was not found.</returns>
        plugin find_plugin(const std::string& name);

        /// <summary>
        /// Returns all benchmarks currently known to the TRRojan.
        /// </summary>
        /// <param name="oit">An output iterator like a back insert
        /// iterator for <see cref="benchmark" />s.</param>
        template<class I> void get_benchmarks(I oit) const;

        /// <summary>
        /// Returns all environments to the given output iterator.
        /// </summary>
        /// <param name="oit">An output iterator like a back insert
        /// iterator.</param>
        template<class I> void get_environments(I oit) const;

        /// <summary>
        /// Answer whether an environment with the given name is available.
        /// </summary>
        /// <param name="name"></param>
        /// <returns></returns>
        inline bool has_environment(const std::string& name) const {
            return (this->environments.find(name) != this->environments.cend());
        }

        /// <summary>
        /// Runs the given JavaScript to conduct benchmarks wiring the results
        /// to the given <paramref name="output" />.
        /// <s/ummary>
        void javascript(const std::string& path, output_base& output,
            const cool_down& coolDown);

        /// <summary>
        /// Loads all plugins in the current directory, retrieves their
        /// environments and initialises those with the given command line.
        /// </summary>
        /// <param name="cmdLine">The command line arguments passed to the
        /// environments.</param>
        void load_plugins(const cmd_line& cmdLine);

        /// <summary>
        /// Runs the given benchmark using the given configurations.
        /// </summary>
        /// <param name="benchmark">The plugin and bechmark to be run.</param>
        /// <param name="configs">The set of configurations to be tested. This
        /// parameter is passed by value because it will be modified by the
        /// method before actually starting the benchmark. For instance, all
        /// names of <see cref="environment" /> and <see cref="device" />s need
        /// to be replaces with their actual instantiation.</param>
        void run(benchmark_base& benchmark, configuration_set configs,
            output_base& output, const cool_down& coolDown,
            const std::size_t continue_at);

        void run(const benchmark& benchmark, const configuration_set& configs,
            output_base& output, const cool_down& coolDown,
            const std::size_t continue_at);

        /// <summary>
        /// Runs the benchmarks in the given TRROLL script writing the results
        /// to the given <paramref name="output" />.
        /// </summary>
        /// <param name="path">The path to the TRROLL script to be executed.
        /// </param>
        /// <param name="output">The output host where the benchmarks put their
        /// results.</param>
        /// <param name="cool_down">Controls regular cool-down periods between
        /// benchmarks that are running over weeks.</param>
        /// <param name="continue_at">Instructs the benchmark to skip all
        /// configurations until the given one.</param>
        /// <param name="power_collector">If not <c>nullptr</c>, enables the
        /// benchmark to measure the power consumption of its work.</param>
        void trroll(const troll_input_type& path,
            output_base& output,
            const cool_down& cool_down,
            const std::size_t continue_at,
            power_collector::pointer power_collector);

        executive operator =(const executive&) = delete;

    private:

        /// <summary>
        /// Holds a possible combination of an environment and devices to be
        /// tested.
        /// </summary>
        /// <remarks>
        /// There might be restrictions on which environment is acceptable on
        /// which device.
        /// </remarks>
        struct env_dev_set {
            trrojan::environment environment;
            std::vector<trrojan::device> devices;

            inline env_dev_set(void) { }

            inline env_dev_set(trrojan::environment&& e, decltype(devices)&& d)
                : environment(std::move(e)), devices(std::move(d)) { }
        };

        /// <summary>
        /// Abstraction of a plugin DLL.
        /// </summary>
        class plugin_dll {

        public:

            /// <summary>
            /// The type of the plugin entry point retrieving the descriptor.
            /// </summary>
            typedef trrojan::plugin_base *(*entry_point_type)(void);

            /// <summary>
            /// The type of a native DLL handle.
            /// </summary>
#if defined(_WIN32)
            typedef HMODULE handle_type;
#else /* defined(_WIN32) */
            typedef void *handle_type;
#endif /* defined(_WIN32) */

            /// <summary>
            /// The type of a native function pointer returned for a resolved
            /// procedure name.
            /// </summary>
#if defined(_WIN32)
            typedef FARPROC proc_type;
#else /* defined(_WIN32) */
            typedef void *proc_type;
#endif /* defined(_WIN32) */

            /// <summary>
            /// Open the DLL at the specified location.
            /// </summary>
            static plugin_dll open(const std::string& path);

            /// <summary>
            /// The name of the entry point which provides us with the
            /// <see cref="trrojan::plugin" /> instance.
            /// </summary>
            static const std::string entry_point_name;

            /// <summary>
            /// The extension (including the leading ".") of a DLL.
            /// </summary>
            static const std::string extension;

            /// <summary>
            /// The value of an invalid handle.
            /// </summary>
            static handle_type invalid_handle;

            /// <summary>
            /// Initialises a new instance.
            /// </summary>
            inline plugin_dll(void) : handle(plugin_dll::invalid_handle) { }

            /// <summary>
            /// Move <paramref name="rhs" />.
            /// </summary>
            inline plugin_dll(plugin_dll&& rhs) noexcept : handle(rhs.handle) {
                rhs.handle = plugin_dll::invalid_handle;
            }

            /// <summary>
            /// Finalises the instance.
            /// </summary>
            ~plugin_dll(void);

            /// <summary>
            /// Closes the library.
            /// </summary>
            void close(void);

            /// <summary>
            /// Finds the procedure with the specified name.
            /// </summary>
            proc_type find(const std::string& name);

            entry_point_type find_entry_point(void) {
                return reinterpret_cast<entry_point_type>(
                    this->find(plugin_dll::entry_point_name));
            }

            /// <summary>
            /// Move assignment.
            /// </summary>
            plugin_dll& operator =(plugin_dll&& rhs) noexcept;

        private:

            /// <summary>
            /// The DLL handle if any.
            /// </summary>
            handle_type handle;
        };

        /// <summary>
        /// Enables the environment with the specified name.
        /// </summary>
        /// <remarks>
        /// Any previously enabled environment stored at
        /// <see cref="cur_environment" /> will be properly deactivated by the
        /// method.
        /// </remarks>
        /// <param name="name"></param>
        /// <exception cref="std::invalid_argument>If the specified environment
        /// does not exist.</exception>
        void enable_environment(const std::string& name);

        /// <summary>
        /// Enables the given environment.
        /// </summary>
        /// <remarks>
        /// Any previously enabled environment stored at
        /// <see cref="cur_environment" /> will be properly deactivated by the
        /// method.
        /// </remarks>
        /// <param name="env"></param>
        /// <exception cref="std::invalid_argument>If <paramref name="env" /> is
        /// <c>nullptr</c>.</exception>
        void enable_environment(environment env);

        /// <summary>
        /// Finds the environment designated by the given string variant, or
        /// return the environment if the variant specified an actual
        /// environment pointer.
        /// </summary>
        /// <param name="v"></param>
        /// <returns></returns>
        environment find_environment(const variant& v);

        /// <summary>
        /// Prepare all possible combinations of <see cref="environment" /> and
        /// <see cref="device"> honouring any restrictions make in in
        /// <paramref name="cs" />.
        /// </summary>
        std::vector<env_dev_set> prepare_env_devs(const configuration_set& cs,
            const std::string& factorEnv = environment_base::factor_name,
            const std::string& factorDev = device_base::factor_name);

        /// <summary>
        /// Stores the currently active environment.
        /// </summary>
        environment cur_environment;

        /// <summary>
        /// Holds all execution environments, indexed by their name.
        /// </summary>
        std::map<std::string, environment> environments;

        /// <summary>
        /// Holds the libraries of all plugins that the application has found.
        /// </summary>
        std::vector<plugin_dll> plugin_dlls;

        /// <summary>
        /// Holds all of the plugin descriptors the application has found.
        /// </summary>
        std::vector<plugin> plugins;

#if defined(TRROJAN_FOR_UWP)
        /// <summary>
        /// Remembers the core window that was created for the application in
        /// order to allow the plugins to draw into this window.
        /// </summary>
        /// <remarks>
        /// <para>In UWP builds, the executive will pass this window to all
        /// benchmarks as <see cref="factor_core_window" />.</para>
        /// </remarks>
        window_type window;
#endif /* defined(TRROJAN_FOR_UWP) */
    };
}

#include "trrojan/executive.inl"
